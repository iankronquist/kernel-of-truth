#ifndef KPUTS_H
#define KPUTS_H

#include <stdarg.h>

// Print a string to the VGA terminal.
void kputs(char *string);
// Print a printf style formatted string to the VGA terminal.
void kprintf(char *string, ...);
// Variable arguments version of kprintf.
void kvprintf(char* string, va_list args);
#endif
