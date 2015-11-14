#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <libk/kputs.h>
#include <arch/x86/io.h>
#include <stdbool.h>

#define KB_STATUS_PORT 0x64
#define KB_DATA_PORT 0x60
#define KB_INTERRUPT_MASK (~2)


unsigned char keyboard_map[256];
unsigned char keyboard_shift_map[256];

void keyboard_irq_handler();
void keyboard_install();

#endif
