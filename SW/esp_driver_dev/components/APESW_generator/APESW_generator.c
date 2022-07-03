#include "APESW_generator.h"

size_t calculate_n_per_GPIO(uint8_t n_inversion, uint8_t n_warmup) {
    return n_inversion * (2*n_warmup + 5) + 2*n_warmup + 1;
}

uint8_t* generate_APESW(
        uint8_t n_inversion,
        uint8_t n_warmup,
        uint8_t n_GPIO,
        APESW_GPIO_Type_t *GPIO_types
        ) {

    // Allocate result
    size_t n_per_GPIO = calculate_n_per_GPIO(n_inversion, n_warmup);
    uint8_t *APESW = malloc(n_per_GPIO * n_GPIO * sizeof(uint8_t));

    // Generate APESW
    size_t idx = 0;
    for (size_t i = 0; i < n_GPIO; i++) {
        APESW_GPIO_Type_t type = GPIO_types[i];

        // j = inversion number
        for (size_t j = 0; j < n_inversion; j++) {
            // k = warmup phase
            for (size_t k = 0; k < 2*n_warmup; k++)
                APESW[idx++] = type.no_warmup ? 0 : k%2 ^ type.is_flipped;
            APESW[idx++] = type.is_flipped;
            APESW[idx++] = type.is_flipped;
            APESW[idx++] = type.is_flipped^1;
            APESW[idx++] = type.is_flipped;
            APESW[idx++] = type.is_flipped^1;

        }
        for (size_t k = 0; k < 2*n_warmup; k++)
            APESW[idx++] = type.no_warmup ? 0 : k%2 ^ type.is_flipped;
        APESW[idx++] = 0;
    }

    return APESW;
}

