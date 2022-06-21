#include "APESW_generator.h"

uint8_t* generate_APESW(
        uint8_t n_inversion,
        uint8_t n_warmup,
        uint8_t n_GPIO,
        APESW_GPIO_Type_t *GPIO_types
        ) {

    // Allocate result
    size_t n_per_GPIO = (n_inversion * (2*n_warmup + 5) + 2*n_warmup);
    uint8_t *APESW = malloc(n_per_GPIO * n_GPIO * sizeof(uint8_t));

    // Generate APESW
    for (size_t i = 0; i < n_GPIO; i++) {
        /* Recipe = Half1 Full1 Half0 Full0 */
        uint8_t recipe = 0b1100;

        APESW_GPIO_Type_t type = GPIO_types[i];
        if (type.is_flipped)
            recipe ^= 0xff;
        if (type.no_warmup)
            recipe &= 0b0101;
    }

    return APESW;
}

