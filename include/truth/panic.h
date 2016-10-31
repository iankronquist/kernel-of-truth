#pragma once

#include <truth/cpu.h>
#include <truth/types.h>
#include <truth/log.h>

#define Not_Reached false

#define assert(x) \
    if (!(x)) { \
        logf("Assertion failed: (%s) function %s, file %s, line %d.\n", \
             (#x), __func__, __FILE__, __LINE__); \
        panic(); \
    }

#define assert_ok(x) \
    if (x != Ok) { \
        logf("Assertion failed (%s) status: %s, function %s, file %s, " \
             "line %d.\n", (#x), status_message(x), __func__, __FILE__, \
             __LINE__); \
        panic(); \
    }


void stack_trace(void);
void panic(void);
