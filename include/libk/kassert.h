#ifndef KASSERT_H
#include "kabort.h"
#include "kputs.h"

void kassert(int value);

/*
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
*/

#endif
