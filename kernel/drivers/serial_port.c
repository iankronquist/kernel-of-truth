#include <drivers/serial_port.h>
#include <arch/x86/io.h>
#include <arch/x86/idt.h>

void initialize_serial_port(int port) {
    // Disable interrupts
    write_port(port + 1, 0x00);
    // Enable divisor mode to set clock rate
    write_port(port + 3, 0x80);
    // Set low bytes of divisor to 115200 baud
    write_port(port + 0, 0x01);
    // Set high bytes of divisor
    write_port(port + 1, 0x00);
    // Disable divisor mode and set parity
    write_port(port + 3, 0x03);
    // Enable FIFO mode and clear buffer
    write_port(port + 2, 0xC7);
    // Enable interrupts
    /*
    write_port(port + 4, 0x0B);
    write_port(port + 1, 0x01);
    */
}

int serial_received(int port) {
    return read_port(port) & 1;
}

char read_serial(int port) {
    while (serial_received(port) == 0);
    return read_port(port);
}

bool is_transmit_empty(int port) {
    return read_port(port + 5) & 0x20;
}

void write_serial(int port, char data) {
    while (is_transmit_empty(port) == 0);
    write_port(port, data);
}

void write_serial_string(int port, char *str) {
    for (size_t i = 0; str[i] != '\0'; ++i) {
        write_serial(port, str[i]);
    }
}
