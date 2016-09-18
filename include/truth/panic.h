#pragma once

#include <truth/cpu.h>
#include <truth/log.h>

#define assert(x) \
    if (!(x)) { \
        logf("Assertion failed: (%s) function %s, file %s, line %d.", \
             (#x), __func__, __FILE__, __LINE__); \
        panic(); \
    }

#define panic() { disable_interrupts(); halt(); }
