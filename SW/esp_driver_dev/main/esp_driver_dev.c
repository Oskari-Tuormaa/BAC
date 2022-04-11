#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

#include <driver/gpio.h>
#include <driver/gptimer.h>
#include "pulser.h"

#define INTR_GPIO 26

static void GPIO_ISR(void* data) {
    static bool steps [] = {
        // V11 - D13
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0,

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

        // V22 - D27
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    pulse_pins(steps, 64);
}

void app_main(void)
{
    gpio_install_isr_service(0);

    /* Setup GPIO interrupt */
    gpio_reset_pin(INTR_GPIO);
    gpio_set_direction(INTR_GPIO, GPIO_MODE_INPUT);
    gpio_set_intr_type(INTR_GPIO, GPIO_INTR_POSEDGE);
    gpio_intr_enable(INTR_GPIO);

    /* Define global GPIO ISR */
    gpio_isr_handler_add(INTR_GPIO, GPIO_ISR, NULL);

    // Setup pulser
    pulser_config_t pulser_config = {
        .gpios   = {13, 12, 14, 27},
        .n_gpio  = 4,
        .freq    = 80e3,
        .divider = 1
    };
    init_pulser(&pulser_config);

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
