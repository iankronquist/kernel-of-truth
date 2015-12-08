#ifndef KASSERT_H
#include "kabort.h"
#include "kputs.h"

#define kassert(value) if (!(value)) { \
    kprintf("Assertion failed: (%s) function %s, file %s, line %d.", #value, __FUNCTION__, __FILE__, __LINE__); \
    kabort();\
}

#endif
