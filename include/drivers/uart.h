#pragma once

#include <truth/device.h>

extern const struct device uart_char_device;

// Register and initialize the uart device.
void register_uart_device(void);
