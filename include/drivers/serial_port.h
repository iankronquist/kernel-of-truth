#pragma once

#include <truth/device.h>

// Register and initialize the serial port device.
void register_serial_port(void);
extern const struct device serial_port_device;
