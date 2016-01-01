#include <drivers/serial_port.h>
#include <arch/x86/io.h>

void write_port(uint8_t value, uint16_t port);

uint8_t read_port(uint16_t port);


void initialize_serial_port(enum serial_port com_port) {
    // For a description of offsets, see Serial Port Controller Manual,
    // Figure 6.
    // http://www.mcamafia.de/pdf/ibm_hitrc08.pdf
    // Disable interrupts
    write_port(com_port + 1, 0x00);
    // Enable DLAB by setting baud rate divisor
    write_port(com_port + 3, 0x80);
    // Set divisor high to 38400 and low to 3
    write_port(com_port + 0, 0x03);
    write_port(com_port + 1, 0x00);
    // 8 bits, no parity, one stop bit
    write_port(com_port + 3, 0x03);
    // Enable FIFO, clear bytes with 14 byte threshold
    write_port(com_port + 2, 0xc7);
    // Enable IRQs and set RTS/DSR
    write_port(com_port + 4, 0x0b);

}

bool is_data_available(enum serial_port com_port) {
    return read_port(com_port + 5) & DATA_AVAIL;
}

// Blocking i/o!
char read_serial(enum serial_port com_port) {
    while (is_data_available(com_port) == false);
    return read_port(com_port);
}

// Blocking i/o!
void read_serial_string(enum serial_port com_port, char *buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        while (is_data_available(com_port) == false);
        buf[i] = read_port(com_port);
    }
}

bool is_transmit_empty(enum serial_port com_port) {
    return !(read_port(com_port + 5) & TRANSMITTER_EMPTY);
}

// Blocking i/o!
void write_serial(enum serial_port com_port, char data) {
    while (is_transmit_empty(com_port) == false);
    write_port(com_port, data);
}


// Blocking i/o!
// Writes a string to the serial port, excluding the null byte.
void write_serial_string(enum serial_port com_port, char *data) {
    for (size_t i = 0;  data[i] != '\0'; ++i) {
    while (is_transmit_empty(com_port) == false);
        write_port(com_port, data[i]);
    }
}
