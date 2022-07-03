#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <esp_log.h>

#include <driver/uart.h>

#include <esp_intr_alloc.h>
#include <sdkconfig.h>
#include <string.h>

#include "pulser.h"
#include "commander.h"
#include "APESW_generator.h"
#include "sys/_stdint.h"

#define INTR_GPIO 26
static const uart_port_t uart_num = UART_NUM_0;
static char buffer[128];
static QueueHandle_t uart_queue;

APESW_GPIO_Type_t gpios[] = {
    {.is_flipped = true, .no_warmup = false},
    {.is_flipped = false, .no_warmup = false},
    {.is_flipped = false, .no_warmup = true},
    {.is_flipped = true, .no_warmup = true},
};

/* Helper functions and ISR {{{*/
static bool is_apesw_allocated = false;
static void* apesw;
static size_t n_steps;
void do_pulse() {
    // static bool const steps [] = {
    //     // V11 - D13
    //     1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
    //     1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
    //     1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
    //     1, 0, 1, 0, 1, 0, 1, 0, 0,

    //     // V21 - D12
    //     0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1,
    //     0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1,
    //     0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1,
    //     0, 1, 0, 1, 0, 1, 0, 1, 0,

    //     // V12 - D14
    //     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    //     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    //     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    //     0, 0, 0, 0, 0, 0, 0, 0, 0,

    //     // V22 - D27
    //     0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
    //     0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
    //     0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
    //     0, 0, 0, 0, 0, 0, 0, 0, 0,
    // };
    // static const uint8_t n_steps = sizeof(steps) / (sizeof(steps[0]) * 4);
    pulse_pins(apesw, n_steps);
    vTaskDelay(portTICK_PERIOD_MS * 4);
}

static void GPIO_ISR(void* data) {
    do_pulse();
}
/*}}}*/

/* Setup functions {{{*/
void setup_interrupt() {
    gpio_install_isr_service(0);

    /* Setup GPIO interrupt */
    gpio_config_t gpio_intr_config = {
        .pin_bit_mask = (1 << INTR_GPIO),
        .intr_type    = GPIO_INTR_POSEDGE,
        .mode         = GPIO_MODE_INPUT,
    };
    gpio_config(&gpio_intr_config);

    /* Define global GPIO ISR */
    gpio_isr_handler_add(INTR_GPIO, GPIO_ISR, NULL);
}

void setup_uart() {
    // Setup UART parameters
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    // Setup UART pins
    ESP_ERROR_CHECK(uart_set_pin(uart_num, 1, 3, -1, -1));

    // Install UART driver
    const int uart_buffer_size = (1024 * 2);
    ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));

    // Install UART interrupt
    uart_enable_rx_intr(uart_num);
}
/*}}}*/

/* Command helper functions {{{*/
void extract_command(char* inbuf, size_t n_inbuf, char* outbuf) {
    // Find space in inbuf
    size_t i_space = 0;
    for (; i_space < n_inbuf && inbuf[i_space] != '\0' && inbuf[i_space] != ' ';i_space++);

    // No space in inbuf
    if (i_space == n_inbuf || inbuf[i_space] == '\0')
        return;

    // Copy string and add null termination
    strncpy(outbuf, inbuf, i_space);
    outbuf[i_space] = '\0';
}

void transmit_header(uint16_t dlen, uint16_t npack) {
    uart_write_bytes(uart_num, &dlen, sizeof(dlen));
    uart_write_bytes(uart_num, &npack, sizeof(npack));
}

void transmit_packet(const void* data, uint16_t dlen) {
    uart_write_bytes(uart_num, data, dlen);
}

void transmit_data(const void* data, uint16_t dlen, uint16_t npack) {
    transmit_header(dlen, npack);
    for (size_t i = 0; i < npack; i++)
        transmit_packet(((char*)data)+i*dlen, dlen);
}
/*}}}*/

/* Command typedefs {{{*/
typedef enum {
    HELLO,
    GENERATE_RAND,
    GET_APESW,
    SET_PARAMS,
    PULSE,
} COMMANDS;

typedef struct {
    COMMANDS command;
    void* data;
} command_t;

typedef struct {
    command_t* cmd;
} callbackdata_t;

typedef struct {
    COMMANDS cmd;
    void (*callback)(callbackdata_t);
} callback_t;
/*}}}*/

/* Command handlers {{{*/
void hello_handler(callbackdata_t cdata) {
    snprintf(buffer, sizeof(buffer), "Hello from ESP32!");
    transmit_data(buffer, strlen(buffer), 1);
}

void generate_rand_handler(callbackdata_t cdata) {
    uint16_t data = *(uint16_t*)cdata.cmd->data;

    long long randnum;
    transmit_header(sizeof(randnum), data);
    for (size_t i = 0; i < data; i++) {
        randnum = rand() % 1000000;
        transmit_packet(&randnum, sizeof(randnum));
    }
}

void get_apesw_handler(callbackdata_t cdata) {
    transmit_data(apesw, n_steps, 4);
}

void set_params_handler(callbackdata_t cdata) {
    if (is_apesw_allocated)
        free(apesw);

    uint8_t *data = cdata.cmd->data;
    uint8_t n_inv = *data;
    uint8_t n_warm = *(data+1);

    apesw = generate_APESW(n_inv, n_warm, 4, gpios);
    n_steps = calculate_n_per_GPIO(n_inv, n_warm);
    is_apesw_allocated = true;

    transmit_data(NULL, 0, 0);
}

void pulse_handler(callbackdata_t cdata) {
    do_pulse();
    transmit_data(NULL, 0, 0);
}

// Register command handlers
#define N_CMD 5
const static callback_t commands[] = {
    {HELLO, hello_handler},
    {GENERATE_RAND, generate_rand_handler},
    {GET_APESW, get_apesw_handler},
    {SET_PARAMS, set_params_handler},
    {PULSE, pulse_handler},
};
/*}}}*/

void app_main(void)
{
    /* Setup {{{*/
    // setup_interrupt();

    // Setup pulser
    pulser_config_t pulser_config = {
        .gpios   = {13, 12, 14, 27},
        .n_gpio  = 4,
        .freq    = 320e3,
        .divider = 4
    };
    init_pulser(&pulser_config);

    // Setup UART
    setup_uart();

    uint8_t n_inv = 3;
    uint8_t n_warm = 6;

    apesw = generate_APESW(n_inv, n_warm, 4, gpios);
    n_steps = calculate_n_per_GPIO(n_inv, n_warm);
    /*}}}*/

    command_t cmd;
    char inbuf[64];
    uart_event_t event;
    while (1) {
        if (xQueueReceive(uart_queue, (void*)&event, portMAX_DELAY)) {
            switch (event.type) {
                case UART_DATA:
                    uart_read_bytes(uart_num, inbuf, event.size, portMAX_DELAY);

                    cmd.command = inbuf[0];
                    cmd.data = &inbuf[1];

                    bool cmd_found = false;
                    for (size_t i = 0; i < N_CMD; i++) {
                        const callback_t cmd_other = commands[i];
                        if (cmd.command == cmd_other.cmd) {
                            cmd_other.callback((callbackdata_t) { &cmd } );
                            cmd_found = true;
                            break;
                        }
                    }
                    if (!cmd_found) {
                        snprintf(buffer, sizeof(buffer), "Unknown command\n");
                        transmit_data(buffer, strlen(buffer), 1);
                    }
                    break;
                default:
                    break;
            }
        }
    }
}
