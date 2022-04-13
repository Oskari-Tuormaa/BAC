/******************************************************************************
* File:             pulser.h
*
* Author:           Oskari Tuormaa
* Created:          04/07/22
*                   Simple driver for pulsing GPIO pins
*****************************************************************************/

#ifndef PULSER_H_
#define PULSER_H_


#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <rom/ets_sys.h>
#include <driver/gptimer.h>


/**
 * @brief Pulser configuration.
 */
typedef struct {
    gpio_num_t gpios[16];
    uint8_t n_gpio;
    uint32_t freq;
    uint32_t divider;

} pulser_config_t;


/**
 * @brief Initializes the pulser driver.
 *
 * @param *config The pulser configuration.
 */
void init_pulser(const pulser_config_t* config);

/**
 * @brief Pulses configured GPIO pins.
 *
 * @note The length of the *steps array must at least be
 *       the amount of GPIO pins multiplied by the amount of steps.
 *
 * @param *steps A boolean array containing the step values.
 * @param n_steps The amount of steps in *steps.
 */
void pulse_pins(const bool* steps, uint8_t n_steps);


#endif /* end of include guard: PULSER_H_9ZT2R8NH */
