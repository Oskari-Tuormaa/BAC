#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

#include <driver/gpio.h>
#include <driver/gptimer.h>
#include "pulser.h"

void app_main(void)
{
    bool steps [] = {
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
    };

    // Setup pulser
    pulser_config_t pulser_config = {
        .gpios = {12, 13, 14, 27},
        .n_gpio = 4,
        .freq = 80e3,
        .divider = 1
    };
    init_pulser(&pulser_config);

    while (1) {
        pulse_pins(steps, 17);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
