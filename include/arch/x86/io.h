#pragma once

#include <truth/types.h>

/* Write to an I/O port.
 * A wrapper around outb. Takes a value to write and a port to write it to.
 */
void write_port(uint8_t value, uint16_t port);
/* Write two bytes to an I/O port. */
void write_port16(uint16_t value, uint16_t port);
/* Write a word to an I/O port. */
void write_port32(uint32_t value, uint16_t port);

/* Read from an I/O port.
 * A wrapper around inb. Takes a port and returns its value.
 */
uint16_t read_port(uint16_t port);
/* Read two bytes from an I/O port. */
uint16_t read_port16(uint16_t port);
/* Read a word from an I/O port. */
uint32_t read_port32(uint16_t port);
