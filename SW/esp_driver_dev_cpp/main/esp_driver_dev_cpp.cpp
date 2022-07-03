#include <memory>
#include <functional>
#include <iostream>
#include <vector>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <rom/ets_sys.h>
#include <driver/gptimer.h>


class SenderDriver {
    public:
        using StepCT = std::vector<bool>;

        SenderDriver(const std::vector<gpio_num_t> &gpios, uint32_t freq, uint32_t divider) : m_gpios(gpios), m_freq(freq), m_divider(divider) {
            // Create pin bit mask
            uint64_t pin_bit_mask = 0;
            for (auto gpio : gpios)
                pin_bit_mask |= 1ULL < gpio;

            // Set GPIO config
            m_gpio_conf.pin_bit_mask = pin_bit_mask;
            m_gpio_conf.mode         = GPIO_MODE_OUTPUT;
            m_gpio_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
            m_gpio_conf.pull_up_en   = GPIO_PULLUP_DISABLE;
            ESP_ERROR_CHECK(gpio_config(&m_gpio_conf));

            /* Setup timer */
            m_timer_config.clk_src       = GPTIMER_CLK_SRC_APB;
            m_timer_config.direction     = GPTIMER_COUNT_UP;
            m_timer_config.resolution_hz = m_freq;
            ESP_ERROR_CHECK(gptimer_new_timer(&m_timer_config, &m_timer_handle));

            /* Setup alarm action */
            m_alarm_config.alarm_count                = m_divider;
            m_alarm_config.reload_count               = 0;
            m_alarm_config.flags.auto_reload_on_alarm = true;
            ESP_ERROR_CHECK(gptimer_set_alarm_action(m_timer_handle, &m_alarm_config));

            /* Setup alarm callback */
            using namespace std::placeholders;
            m_callback_config.on_alarm = &SenderDriver::alarm_callback;
            ESP_ERROR_CHECK(gptimer_register_event_callbacks(m_timer_handle, &m_callback_config, &m_callback_data));
        }

        void pulse_pins(const std::vector<bool> *steps) {
            m_callback_data.steps = steps;
            gptimer_start(m_timer_handle);
        }

        static bool alarm_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
            static size_t step = 0;

            return false;
        }

    private:
        /* Configuration variables */
        std::vector<gpio_num_t> m_gpios;
        uint32_t m_freq;
        uint32_t m_divider;

        /* ESP-IDF internal variables */
        gpio_config_t             m_gpio_conf;
        gptimer_event_callbacks_t m_callback_config;
        gptimer_alarm_config_t    m_alarm_config;
        gptimer_config_t          m_timer_config;
        gptimer_handle_t          m_timer_handle;
        struct {
            const StepCT * steps;
        } m_callback_data;
};

extern "C" void app_main(void) {
    std::vector<gpio_num_t> gpios{
        GPIO_NUM_12,
        GPIO_NUM_13,
        GPIO_NUM_14,
        GPIO_NUM_21,
    };
    SenderDriver sd(gpios, 40e3, 1);
}
