#include <drivers/serial_port.h>
#include <arch/x86/io.h>
#include <arch/x86/idt.h>

#define SERIAL_1 0x3f8
#define SERIAL_2 0x2f8

// Constants for the Received Data Status Register
#define DATA_CARRIER_DETECT (1<<7)
#define CLEAR_TO_SEND (1<<6)
#define DATA_SET_READY (1<<5)
#define BREAK (1<<4)
#define FRAMING (1<<3)
#define PARITY_ERROR (1<<2)
#define OVERRUN_ERROR (1<<1)
#define ERROR_OR_BREAK (1<<0)

// Serial Port Compatible Register Address Offsets
#define TRANSMIT_HOLDING(x) (x+0)
#define RECEIVER_BUFFER(x) (x+0)
#define DIVISOR_LATCH_LOW(x) (x+0)
#define DIVISOR_LATCH_HIGH(x) (x+1)
#define INTERRUPT_ENABLE(x) (x+1)
#define INTERRUPT_ID(x) (x+2)
#define FIFO_CONTROL(x) (x+2)
#define LINE_CONTROL(x) (x+3)
#define MODEM_CONTROL(x) (x+4)
#define LINE_STATUS(x) (x+5)
#define MODUM_STATUS(x) (x+6)
#define SCRATCH(x) (x+7)

void outportb(unsigned short _port, unsigned char _data) {
	__asm__ volatile ("outb %1, %0" : : "dn" (_port), "a" (_data));
}

unsigned char inportb(unsigned short _port) {
	unsigned char rv;
	__asm__ volatile ("inb %1, %0" : "=a" (rv) : "dn" (_port));
	return rv;
}


void initialize_serial_port(int port) {
    outportb(port + 1, 0x00); /* Disable interrupts */
    outportb(port + 3, 0x80); /* Enable divisor mode */
    outportb(port + 0, 0x01); /* Div Low:  01 Set the port to 115200 bps */
    outportb(port + 1, 0x00); /* Div High: 00 */
    outportb(port + 3, 0x03); /* Disable divisor mode, set parity */
    outportb(port + 2, 0xC7); /* Enable FIFO and clear */
    outportb(port + 4, 0x0B); /* Enable interrupts */
    outportb(port + 1, 0x01); /* Enable interrupts */
}

int serial_received(int port) {
    return inportb(port) & 1;
}

char read_serial(int port) {
    while (serial_received(port) == 0);
    return inportb(port);
}

bool is_transmit_empty(int port) {
    return inportb(port + 5) & 0x20;
}

void write_serial(int port, char data) {
    while (is_transmit_empty(port) == 0);
    outportb(port, data);
}

void write_serial_string(int port, char *str) {
    for (size_t i = 0; str[i] != '\0'; ++i) {
        write_serial(port, str[i]);
    }
}
