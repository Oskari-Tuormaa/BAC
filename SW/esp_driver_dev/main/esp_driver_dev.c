#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <esp_log.h>

#include <driver/uart.h>

#include <esp_intr_alloc.h>
#include <sdkconfig.h>
#include <string.h>

#include "pulser.h"
#include "APESW_generator.h"

#define INTR_GPIO 26
static const uart_port_t uart_num = UART_NUM_0;
static QueueHandle_t uart_queue;

void do_pulse() {
    static bool const steps [] = {
        // V11 - D13
        1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 0,

        // V21 - D12
        0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0,

        // V12 - D14
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0,

        // V22 - D27
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    static const uint8_t n_steps = sizeof(steps) / (sizeof(steps[0]) * 4);
    pulse_pins(steps, n_steps);
}

static void GPIO_ISR(void* data) {
    do_pulse();
}

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

void app_main(void)
{
    // setup_interrupt();

    // Setup pulser
    pulser_config_t pulser_config = {
        .gpios   = {13, 12, 14, 27},
        .n_gpio  = 4,
        .freq    = 80e3,
        .divider = 1
    };
    init_pulser(&pulser_config);

    // Setup UART
    setup_uart();

    char cmd[128];
    uart_event_t event;
    while (1) {
        if (xQueueReceive(uart_queue, (void*)&event, portMAX_DELAY)) {
            switch (event.type) {
                case UART_DATA:
                    uart_read_bytes(uart_num, cmd, event.size, portMAX_DELAY);
                    if (cmd[0] == 'p') {
                        do_pulse();
                    }
                    break;
                default:
                    break;
            }
        }
    }
}
