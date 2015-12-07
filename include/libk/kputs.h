#ifndef KPUTS_H
#define KPUTS_H
#include <stdarg.h>
#include "drivers/terminal.h"

void kputs(char* string);
void kprintf(char* string, ...);

#endif
