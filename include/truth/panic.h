#pragma once

#include <truth/cpu.h>
#include <truth/interrupts.h>
#include <truth/log.h>
#include <truth/types.h>

#define Not_Reached false

#define assert(x) \
    if (!(x)) { \
        logf(Log_Error, \
             "Assertion failed: (%s) function %s, file %s, line %d.", \
             (#x), __func__, __FILE__, __LINE__); \
        panic(); \
    }

#define assert_ok(x) { \
        enum status __status = x; \
        if (__status != Ok) { \
            logf(Log_Error, \
                 "Assertion failed: (%s) did not return Ok.\n" \
                 "Status: %s, function %s, file %s, line %d.", \
                 (#x), status_message(__status), __func__, __FILE__, __LINE__); \
            panic(); \
        } \
    }

static inline void panic() {
    interrupts_disable();
    halt();
}
