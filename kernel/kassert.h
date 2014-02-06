#include "../tlibc/stdio/stdio.h"
#include "kabort.c"

//Begin assert macro
#define assert(condition) \
	if (! condition) \
	{ \
		kputs("ASSERTION FAILED"); \
		kputs("asserted that "  #condition ""); \
		kabort(); \
	} \
//End macro

