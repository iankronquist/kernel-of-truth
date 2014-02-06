#if defined(__linux__)
	#include <stdio.h>
#else
	#include "stdio/stdio.h"
#endif

void abort(void)
{
	// TODO: Add proper kernel panic.
	printf("Kernel Panic! abort()\n");
	while ( 1 ) { }
}

