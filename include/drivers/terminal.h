#pragma once

#include <truth/device.h>

extern struct device *terminal_char_device;
// Register and initialize the VGA terminal device.
void terminal_initialize(void);
