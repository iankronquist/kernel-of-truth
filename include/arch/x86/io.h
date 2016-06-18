#ifndef IO_H
#define IO_H
#include <stdint.h>

/* Write to an I/O port.
 * A wrapper around outb. Takes a @value to write and a @port to write it to.
 */
void write_port(uint16_t value, uint16_t port);

/* Read from an I/O port.
 * A wrapper around inb. Takes a @port and returns its value.
 */
uint16_t read_port(uint16_t port);

#endif
