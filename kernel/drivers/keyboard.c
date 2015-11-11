#include <drivers/keyboard.h>

void keyboard_install() {
    // Enable only IRQ1
    write_port(0x21 , 0xFD);
}


void keyboard_irq_handler() {
    unsigned char status;
    char key_code;

    // End interrupt
    write_port(0x20, 0x20);

    // Read Keyboard status form port
    status = read_port(KB_STATUS_PORT);

    if (status & 1) {
        // Read in from KB_DATA_PORT into key_code
        key_code = read_port(KB_DATA_PORT);
        if (key_code < 0) {
            return;
        } else {
            term_putchar(key_code);
        }
    }

}
