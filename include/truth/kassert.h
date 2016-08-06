#ifndef KASSERT_H
#include <truth/kabort.h>
#include <truth/kputs.h>

// An assertion macro. Print the location of the failure to the VGA terminal
// and kernel panic.
#define kassert(value) if (!(value)) { \
    kprintf("Assertion failed: (%s) function %s, file %s, line %d.", #value, __FUNCTION__, __FILE__, __LINE__); \
    kabort();\
}

#endif
