/******************************************************************************
* File:             pulser.h
*
* Author:           Oskari Tuormaa  
* Created:          04/07/22 
*                   Simple driver for pulsing GPIO pins
*****************************************************************************/

#ifndef PULSER_H_9ZT2R8NH
#define PULSER_H_9ZT2R8NH


#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <rom/ets_sys.h>
#include <driver/gptimer.h>


typedef struct {
    gpio_num_t gpios[16];
    uint8_t n_gpio;
    uint32_t freq;
    uint32_t divider;

} pulser_config_t;


void init_pulser(pulser_config_t* config);
void pulse_pins(bool* steps, uint8_t n_steps);


#endif /* end of include guard: PULSER_H_9ZT2R8NH */
