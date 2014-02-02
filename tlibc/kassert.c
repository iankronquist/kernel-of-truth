#include "stdlib.h"

//Begin assert macro
#define assert(condition) \
	if (! condition) \
	{ \
		puts("ASSERTION FAILED"); \
		puts("asserted that "  #condition ""); \
		/*printf("a %p", &a); \
		printf("c %p", &c); */\
		abort(); \
	} \
//End macro

