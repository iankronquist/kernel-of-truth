#ifndef KPUTS_C
#define KPUTS_C

#include "terminal.h"

void kputs(char* string)
{
	term_writestring(string);
}

#endif
