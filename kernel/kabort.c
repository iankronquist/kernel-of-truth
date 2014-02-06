#include "kabort.h"

void kabort(void)
{
	// TODO: Add proper kernel panic.
	kputs("Kernel Panic! abort()\n");
	while ( 1 ) { }
}

