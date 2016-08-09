#include <arch/x86/io.h>

#include <drivers/serial_port.h>

#include <truth/interrupts.h>
#include <truth/types.h>

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


static enum COM_PORT ports[] = { COM1, COM2, COM3, COM4 };

void serial_fini(const struct device *unused(dev));

static void initialize_serial_port(enum COM_PORT port) {
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

static char read_serial(enum COM_PORT port) {
    while (serial_received(port) == 0);
    return read_port(port);
}

static inline bool is_transmit_empty(enum COM_PORT port) {
    return read_port(port + 5) & 0x20;
}

static void write_serial(enum COM_PORT port, char data) {
    while (is_transmit_empty(port) == 0);
    write_port16(port, data);
}

status_t checked serial_init(const struct device *unused(dev),
        int device_number, void *unused(args)) {
    initialize_serial_port(ports[device_number]);
    return Ok;
}


ssize_t serial_puts(const struct device *dev, char *buf, size_t size) {
    size_t i;
    for (i = 0; i < size; ++i) {
        write_serial(ports[dev->number-1], buf[i]);
    }
    return i;
}

ssize_t serial_gets(const struct device *dev, char *buf, size_t size) {
    size_t i;
    for (i = 0; i < size; ++i) {
        buf[i] = read_serial(ports[dev->number-1]);
    }
    return i;
}

int serial_putc(const struct device *dev, char c) {
    write_serial(ports[dev->number-1], c);
    return c;
}

int serial_getc(const struct device *dev) {
    return read_serial(ports[dev->number-1]);
}

void register_serial_port(void) {
}

#define DEFINE_COM(n) \
const struct device serial_com_##n = { \
    .name = "COM" #n, \
    .number = n, \
    .type = char_device, \
    .character = { \
        .init = serial_init, \
        .fini = serial_fini, \
        .read_char = serial_getc, \
        .read = serial_gets, \
        .write = serial_puts, \
        .write_char = serial_putc, \
    } \
};

DEFINE_COM(1);
DEFINE_COM(2);
DEFINE_COM(3);
DEFINE_COM(4);

void serial_fini(const struct device *unused(dev)) {
    unregister_device(&serial_com_1);
    unregister_device(&serial_com_2);
    unregister_device(&serial_com_3);
    unregister_device(&serial_com_4);
}
const struct device *klog_char_device = &serial_com_1;
