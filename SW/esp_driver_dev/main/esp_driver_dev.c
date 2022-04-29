#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

#include <driver/gpio.h>
#include <driver/gptimer.h>
#include "pulser.h"

#define INTR_GPIO 26

static void GPIO_ISR(void* data) {
    static bool const steps [] = {
        // V22 - D13
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

        // V21 - D12
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,

        // V12 - D14
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

        // V11 - D27
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0,
    };
    uint8_t n_steps = sizeof(steps) / (sizeof(steps[0]) * 4);
    pulse_pins(steps, n_steps);
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

void app_main(void)
{
    setup_interrupt();

    // Setup pulser
    pulser_config_t pulser_config = {
        .gpios   = {13, 12, 14, 27},
        .n_gpio  = 4,
        .freq    = 80e3,
        .divider = 1
    };
    init_pulser(&pulser_config);

    // bool const steps [] = {
    //     // V11 - D13
    //     1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
    //     1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
    //     1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
    //     1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0,

    //     // V21 - D12
    //     0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1,
    //     0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1,
    //     0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1,
    //     0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,

    //     // V12 - D14
    //     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    //     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    //     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    //     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    //     // V22 - D27
    //     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
    //     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
    //     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
    //     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // };
    // uint8_t n_steps = sizeof(steps) / (sizeof(steps[0]) * 4);

    while (1) {
        // pulse_pins(steps, n_steps);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
