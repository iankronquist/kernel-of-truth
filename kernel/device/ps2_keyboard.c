#include <truth/device/vga.h>
#include <truth/interrupts.h>
#include <truth/file.h>
#include <truth/log.h>

#include <arch/x64/port.h>

#define Keyboard_Right_Shift_Held 0x36
#define Keyboard_Right_Shift_Released 0xb6
#define Keyboard_Left_Shift_Held 0x2a
#define Keyboard_Left_Shift_Released 0xaa
#define Keyboard_Input_Available 0x1

#define Keyboard_Status_Port 0x64
#define Keyboard_Data_Port 0x60

#define Keyboard_IRQ_Number 0x21



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


struct file Keyboard_Device = {
    .type = File_Device,
    .name = "keyboard",
    .permissions = 0744,
    .obj = Object_Clear,
};


bool keyboard_handler(struct interrupt_cpu_state *unused(r)) {
    static bool shift_held = false;
    uint8_t status_port;
    uint8_t key_code;
    char c = 0;

    status_port = read_port(Keyboard_Status_Port);

    if (status_port & Keyboard_Input_Available) {
        key_code = read_port(Keyboard_Data_Port);
        if (key_code == Keyboard_Right_Shift_Held ||
                key_code == Keyboard_Right_Shift_Held) {
            shift_held = true;
            return true;
        } else if (key_code == Keyboard_Right_Shift_Released ||
                    key_code == Keyboard_Left_Shift_Released) {
            shift_held = false;
            return true;
        } else {
            if (shift_held) {
                c = keyboard_shift_map[key_code];
            } else {
                c = keyboard_map[key_code];
            }
        }
        if (c != 0) {
            vga_putc(c);
        }
    }

    return true;
}


void keyboard_init(void) {
    file_attach_path("/dev/", &Keyboard_Device);
    interrupt_register_handler(Keyboard_IRQ_Number, keyboard_handler);
}


void keyboard_fini(void) {
    interrupt_unregister_handler(Keyboard_IRQ_Number, keyboard_handler);
    file_fini(&Keyboard_Device);
}
