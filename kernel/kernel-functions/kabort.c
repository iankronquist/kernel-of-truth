#include "kabort.h"

void kabort()
{
	kputs("\nKernel Panic! Aborting!\n");
    __asm__("cli; hlt;");
	while ( 1 ) { }
}
