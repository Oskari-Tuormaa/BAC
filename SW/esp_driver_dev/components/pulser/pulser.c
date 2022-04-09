#include "pulser.h"

#define PNAME "PLSR"


/** Driver private structs **/
typedef struct {
    bool* steps;
    uint8_t n_steps;
} callback_data_t;


/** Driver variables **/
pulser_config_t* m_config;
gptimer_event_callbacks_t m_callback_config;
gptimer_alarm_config_t m_alarm_config;
gptimer_config_t m_timer_config;
gptimer_handle_t m_timer_handle;
callback_data_t m_callback_data;


/** Private functions **/
static bool alarm_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
    static size_t step = 0;
    callback_data_t* data = (callback_data_t*)user_ctx;

    /* Set GPIO values */
    for (size_t i = 0; i < m_config->n_gpio; ++i) {
        bool val = *((data->steps + data->n_steps * i) + step);
        gpio_set_level(m_config->gpios[i], val);
    }

    /* Increment step */
    step = (step + 1) % data->n_steps;

    /* Check stop condition */
    if (step == 0)
        gptimer_stop(timer);

    return false;
}


/** Public functions **/
void init_pulser(pulser_config_t* config) {
    m_config = config;

    /* Setup GPIO pins */
    for (size_t i = 0; i < m_config->n_gpio; i++) {
        gpio_num_t gpio = m_config->gpios[i];
        ESP_LOGI(PNAME, "Setting up GPIO%d", gpio);

        ESP_ERROR_CHECK(gpio_reset_pin(gpio));
        ESP_ERROR_CHECK(gpio_set_direction(gpio, GPIO_MODE_OUTPUT));
    }

    /* Setup timer */
    m_timer_config.clk_src       = GPTIMER_CLK_SRC_APB;
    m_timer_config.direction     = GPTIMER_COUNT_UP;
    m_timer_config.resolution_hz = m_config->freq;
    ESP_ERROR_CHECK(gptimer_new_timer(&m_timer_config, &m_timer_handle));

    /* Setup alarm action */
    m_alarm_config.alarm_count                = m_config->divider;
    m_alarm_config.reload_count               = 0;
    m_alarm_config.flags.auto_reload_on_alarm = true;
    ESP_ERROR_CHECK(gptimer_set_alarm_action(m_timer_handle, &m_alarm_config));

    /* Setup alarm callback */
    m_callback_config.on_alarm = alarm_callback;
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(m_timer_handle, &m_callback_config, &m_callback_data));
}


void pulse_pins(bool* steps, uint8_t n_steps) {
    /* Setup steps */
    m_callback_data.steps = steps;
    m_callback_data.n_steps = n_steps;

    /* Start timer */
    gptimer_start(m_timer_handle);
}
