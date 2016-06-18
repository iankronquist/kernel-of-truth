#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <libk/kputs.h>
#include <arch/x86/io.h>
#include <arch/x86/idt.h>
#include <stdbool.h>

#define KB_STATUS_PORT 0x64
#define KB_DATA_PORT 0x60
#define KB_INTERRUPT_MASK (~2)

/* Install the keyboard driver */
void keyboard_install(void);

#endif
