#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <stdbool.h>
#include <stddef.h>

#define NO_PARITY (0<<3)
#define EVEN_PARITY ((1<<3)|(0<<4)|(0<<5))
#define ODD_PARITY ((1<<3)|(1<<4)|(0<<5))
#define HIGH_PARITY ((1<<3)|(0<<4)|(1<<5))
#define LOW_PARITY ((1<<3)|(1<<4)|(1<<5))

#define ONE_STOP_BIT (0<<2)
#define TWO_STOP_BITS (1<<2)

#define DATA_BITS_5 (0<<0)|(0<<1)
#define DATA_BITS_6 (1<<0)|(0<<1)
#define DATA_BITS_7 (0<<0)|(1<<1)
#define DATA_BITS_8 (1<<0)|(1<<1)

#define DATA_AVAIL 0x1
#define TRANSMITTER_EMPTY 0x2
#define ERROR 0x4
#define STATUS_CHANGE 0x8

// The 4 available serial ports.
enum COM_PORT {
    COM1 = 0x3f8,
    COM2 = 0x2f8,
    COM3 = 0x3e8,
    COM4 = 0x2e8,
};


// Initialize the serial port
void initialize_serial_port(enum COM_PORT com_port);

// Read one byte from @port
char read_serial(enum COM_PORT port);

// Write one byte of @data to @port
void write_serial(enum COM_PORT port, char data);

// Write a string of @data to the @port, not including the NULL terminator.
void write_serial_string(enum COM_PORT port, char *data);

// Read @len bytes from @port and store them in the @buffer.
void read_serial_string(enum COM_PORT port, char *buffer, size_t len);

#endif
