#ifndef KPUTS_H
#define KPUTS_H
#include <stdarg.h>

#ifdef ARCH_X86
#include "drivers/terminal.h"
#endif

// Print a string to the VGA terminal.
void kputs(char *string);
// Print a printf style formatted string to the VGA terminal.
void kprintf(char *string, ...);

#endif
