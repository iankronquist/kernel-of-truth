#include <drivers/keyboard.h>
#include <drivers/terminal.h>

#include <arch/x86/io.h>

// This table was shamelessly stolen from:
// http://www.osdever.net/bkerndev/Docs/keyboard.htm
static unsigned char keyboard_map[256] = {
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

static unsigned char keyboard_shift_map[256] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*',    /* 9 */
    '(', ')', '_', '+', '\b',    /* Backspace */
    '\t',            /* Tab */
    'Q', 'W', 'E', 'R',    /* 18 */
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',    /* Enter key */
    0,            /* 29   - Control */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',    /* 39 */
    '"', '~',   0,        /* Left shift */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N',            /* 49 */
    'M', '<', '>', '?',   0,                /* Right shift */
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
    '_',
    0,    /* Left Arrow */
    0,
    0,    /* Right Arrow */
    '=',
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

void keyboard_irq_handler(struct cpu_state *unused(r)) {
    static bool shift_held = false;
    uint8_t status;
    uint8_t key_code;
    char c = 0;

    // End interrupt
    write_port(0x20, 0x20);

    // Read Keyboard status form port
    status = read_port(KB_STATUS_PORT);

    if (status & 1) {
        // Read in from KB_DATA_PORT into key_code
        key_code = read_port(KB_DATA_PORT);
        if (key_code == 0x2a || key_code == 0x36) {
            // If shift is pressed
            shift_held = true;
            return;
        } else if (key_code == 0xaa || key_code == 0xb6) {
            // If shift is released
            shift_held = false;
            return;
        } else {
            if (shift_held) {
                c = keyboard_shift_map[key_code];
            } else {
                c = keyboard_map[key_code];
            }
        }
        if (c != 0)
            terminal_putchar(c);
    }

}

void keyboard_install(void) {
    uint8_t current_mask = read_port(0x21);
    write_port(0x21 , current_mask & KB_INTERRUPT_MASK);
    install_interrupt(33, keyboard_irq_handler);
}
