#include <arch/x86/io.h>

#include <drivers/serial_port.h>

#include <truth/interrupts.h>

void initialize_serial_port(enum COM_PORT port) {
    // Disable interrupts
    write_port16(port + 1, 0x00);
    // Enable divisor mode to set clock rate
    write_port16(port + 3, 0x80);
    // Set low bytes of divisor to 115200 baud
    write_port16(port + 0, 0x01);
    // Set high bytes of divisor
    write_port16(port + 1, 0x00);
    // Disable divisor mode and set parity
    write_port16(port + 3, 0x03);
    // Enable FIFO mode and clear buffer
    write_port16(port + 2, 0xC7);
    // Enable interrupts
    write_port16(port + 4, 0x0B);
    write_port16(port + 1, 0x01);
}

static inline bool serial_received(enum COM_PORT port) {
    return read_port(port) & 1;
}

char read_serial(enum COM_PORT port) {
    while (serial_received(port) == 0);
    return read_port(port);
}

static inline bool is_transmit_empty(enum COM_PORT port) {
    return read_port(port + 5) & 0x20;
}

void write_serial(enum COM_PORT port, char data) {
    while (is_transmit_empty(port) == 0);
    write_port16(port, data);
}

void write_serial_string(enum COM_PORT port, char *str) {
    for (size_t i = 0; str[i] != '\0'; ++i) {
        while ((is_transmit_empty(port)) == 0);
        write_port16(port, str[i]);
    }
}

void read_serial_string(enum COM_PORT port, char *buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        while (serial_received(port) == 0);
        buf[i] = read_serial(port);
    }
}
