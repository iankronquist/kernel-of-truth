#pragma once

#include <truth/cpu.h>
#include <truth/log.h>

#define not_implemented(x) { \
        logf(Log_Error, "%s is not yet implemented\n", x); \
        logf(Log_Error, "Pull requests are welcome! " \
                "Visit %s to learn more.\n", \
                project_website); \
        logf(Log_Error, "Function %s, file %s, line %d.", __func__, __FILE__, \
                __LINE__); \
        panic(); \
    }

#define assert(x) \
    if (!(x)) { \
        logf(Log_Error, \
                "Assertion failed: (%s) function %s, file %s, line %d.", \
                (#x), __func__, __FILE__, __LINE__); \
        panic(); \
    }

#define panic() { disable_interrupts(); halt(); }
