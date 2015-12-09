#ifndef KPUTS_H
#define KPUTS_H
#include <stdarg.h>

#ifdef ARCH_X86
#include "drivers/terminal.h"
#endif

void kputs(char* string);
void kprintf(char* string, ...);

#endif
