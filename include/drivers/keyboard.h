#pragma once

#include <truth/interrupts.h>

#define KB_STATUS_PORT 0x64
#define KB_DATA_PORT 0x60
#define KB_INTERRUPT_MASK (~2)

/* Install the keyboard driver */
void keyboard_install(void);
