#ifndef IO_H
#define IO_H
#include <stdint.h>

void write_port(uint16_t value, uint16_t port);

uint16_t read_port(uint16_t port);

#endif
