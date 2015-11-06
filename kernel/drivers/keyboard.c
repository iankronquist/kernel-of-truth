#include <drivers/keyboard.h>

void keyboard_irq_handler() {
    unsigned char status;
    char key_code;

    uint8_t value = 0x20;
    uint16_t port = 0x20;
    // End interrupt
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port) );

    // Read Keyboard status form port
    __asm__ volatile ( "inb %1, %0" : "=a"(status) : "Nd"(KB_STATUS_PORT) );

    if (status & 1) {
        // Read in from KB_DATA_PORT into key_code
        __asm__ volatile ( "inb %1, %0" : "=a"(key_code) : "Nd"(KB_DATA_PORT) );
        if (key_code < 0) {
            return;
        } else {
            kprint_int("keypress: ", key_code);
        }
    }

}
