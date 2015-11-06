#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <kputs.h>

#define KB_STATUS_PORT 0x60
#define KB_DATA_PORT 0x64

void keyboard_irq_handler();

#endif
