#include <stdbool.h>
#include <stdio.h>

#ifndef _APESW_GENERATOR_H
#define _APESW_GENERATOR_H 

typedef struct {
    bool is_flipped;
    bool no_warmup;
} APESW_GPIO_Type_t;

size_t calculate_n_per_GPIO(uint8_t n_inversion, uint8_t n_warmup);

uint8_t* generate_APESW(
        uint8_t n_inversion,
        uint8_t n_warmup,
        uint8_t n_GPIO,
        APESW_GPIO_Type_t *GPIO_types
    );

#endif
