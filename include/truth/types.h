#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// A boolean status for functions which may fail.
enum status {
    Err = 0,
    Ok = 1,
};

// A boolean status for functions which may fail.
typedef enum status status_t;

#define checked __attribute__((warn_unused_result))
#define unused(x) x __attribute__((unused))

// A physical address.
typedef uintptr_t page_frame_t;
