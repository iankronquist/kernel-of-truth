#ifndef KASSERT_H
#include "kabort.h"
#include "kputs.h"

#define kassert(value) if (!(value)) { \
    kputs("Assertion failed: (#value) function __FUNCTION__, file __FILE__, line __LINE__."); \
    kabort();\
}

#endif
