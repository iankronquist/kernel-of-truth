#ifndef KPUTS_H
#define KPUTS_H
#include <stdarg.h>

#ifdef ARCH_X86
#include "drivers/terminal.h"
#endif

void sys_kputs(char* string);
void sys_kprintf(char* string, ...);

#endif
