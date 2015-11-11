#include <drivers/keyboard.h>

// This table was shamelessly stolen from:
// http://www.osdever.net/bkerndev/Docs/keyboard.htm
unsigned char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',    /* 9 */
    '9', '0', '-', '=', '\b',    /* Backspace */
    '\t',            /* Tab */
    'q', 'w', 'e', 'r',    /* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',    /* Enter key */
    0,            /* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',    /* 39 */
    '\'', '`',   0,        /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',            /* 49 */
    'm', ',', '.', '/',   0,                /* Right shift */
    '*',
    0,    /* Alt */
    ' ',    /* Space bar */
    0,    /* Caps lock */
    0,    /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,    /* < ... F10 */
    0,    /* 69 - Num lock*/
    0,    /* Scroll Lock */
    0,    /* Home key */
    0,    /* Up Arrow */
    0,    /* Page Up */
    '-',
    0,    /* Left Arrow */
    0,
    0,    /* Right Arrow */
    '+',
    0,    /* 79 - End key*/
    0,    /* Down Arrow */
    0,    /* Page Down */
    0,    /* Insert Key */
    0,    /* Delete Key */
    0,   0,   0,
    0,    /* F11 Key */
    0,    /* F12 Key */
    0,    /* All other keys are undefined */
};


void keyboard_install() {
    // Enable only IRQ1
    write_port(0x21 , 0xFD);
}


void keyboard_irq_handler() {
    unsigned char status;
    int8_t key_code;

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
            term_putchar(keyboard_map[key_code]);
        }
    }

}
