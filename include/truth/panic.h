#pragma once

#include <truth/cpu.h>
#include <truth/types.h>
#include <truth/log.h>

#define Not_Reached false

#define assert(x) \
    if (!(x)) { \
        logf(Log_Error, \
             "Assertion failed: (%s) function %s, file %s, line %d.", \
             (#x), __func__, __FILE__, __LINE__); \
        panic(); \
    }

#define assert_ok(x) \
    if (x != Ok) { \
        logf(Log_Error, \
             "Assertion failed (%s) status: %s, function %s, file %s, " \
             "line %d.", (#x), status_message(x), __func__, __FILE__, \
             __LINE__); \
        panic(); \
    }

static inline void panic() {
    disable_interrupts();
    halt();
}
