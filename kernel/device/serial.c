#include <arch/x64/port.h>
#include <truth/file.h>

// The 4 available serial ports.
enum com_port {
    COM1 = 0x3f8,
    COM2 = 0x2f8,
    COM3 = 0x3e8,
    COM4 = 0x2e8,
};

static enum status init_port(enum com_port port) {
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
    write_port(port + 2, 0xc7);
    // Enable interrupts
    write_port(port + 4, 0x0b);
    write_port(port + 1, 0x01);

    return Ok;
}

static inline bool serial_received(enum com_port port) {
    return read_port(port) & 1;
}

static uint8_t read_serial_byte(enum com_port port) {
    while (serial_received(port) == 0);
    return read_port(port);
}

static enum status checked serial_read(enum com_port port, uint8_t *buf,
        size_t size) {
    size_t i;
    for (i = 0; i < size; ++i) {
        buf[i] = read_serial_byte(port);
    }
    return Ok;
}

static inline bool is_transmit_empty(enum com_port port) {
    return read_port(port + 5) & 0x20;
}

static void write_serial_byte(enum com_port port, uint8_t data) {
    while (is_transmit_empty(port) == 0);
    write_port(data, port);
}

static enum status checked serial_write(enum com_port port, const uint8_t *buf,
        size_t size) {
    size_t i;
    for (i = 0; i < size; ++i) {
        write_serial_byte(port, buf[i]);
    }
    return Ok;
}

#define serial_file(id) \
static enum status checked serial_init_##id(char *file_name); \
static enum status checked serial_write_##id(const uint8_t *data, size_t size); \
static enum status checked serial_read_##id(uint8_t *data, size_t size); \
struct file id##_File = { \
    .name = NULL, \
    .init = serial_init_##id, \
    .fini = NULL, \
    .read = serial_read_##id, \
    .write = serial_write_##id, \
    .parent = NULL, \
    .children = NULL, \
    .references = 0, \
    .permissions = Perm_Write, \
}; \
static enum status checked serial_init_##id(char *file_name) { \
    id##_File.name = file_name; \
    return init_port(id); \
} \
static enum status checked serial_write_##id(const uint8_t *in, size_t size) { \
    return serial_write(id, in, size); \
} \
static enum status checked serial_read_##id(uint8_t *in, size_t size) { \
    return serial_read(id, in, size); \
}

serial_file(COM1)

struct file *Log_File = &COM1_File;
