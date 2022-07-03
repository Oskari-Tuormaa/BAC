#include "stdlib.h"

typedef struct {
    uint16_t n_data;
    void* data;
} CallbackData_t;

void register_command_handler(size_t, void(*)(CallbackData_t));
void handle_command();
