#ifndef KABORT_C
#define KABORT_C
#include "kputs.c"

void kabort(void)
{
	// TODO: Add proper kernel panic.
	kputs("Kernel Panic! abort()\n");
	while ( 1 ) { }
}

#endif
