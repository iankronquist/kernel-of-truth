#include "kputs.c"

void abort(void)
{
	// TODO: Add proper kernel panic.
	kputs("Kernel Panic! abort()\n");
	while ( 1 ) { }
}

