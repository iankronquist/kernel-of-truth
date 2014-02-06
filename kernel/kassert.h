#include "../tlibc/stdio/stdio.h"
//#include "kabort.h"
//#include "kputs.h"

//Begin assert macro
#define assert(condition) \
	if (! condition) \
	{ \
		kputs("ASSERTION FAILED"); \
		kputs("asserted that "  #condition ""); \
		kabort(); \
	} \
//End macro

