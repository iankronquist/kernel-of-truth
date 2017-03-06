#pragma once

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef uint64_t phys_addr;

enum partial {
    Partial_Equal,
    Partial_Not_Equal,
};

enum order {
    Order_Equal,
    Order_Greater,
    Order_Less,
};

enum permissions {
    Perm_Read = 1,
    Perm_Write = 2,
    Perm_Execute = 3,
};

enum status {
    Ok,
    Error_No_Memory,
    Error_Invalid,
    Error_Permissions,
    Error_Present,
    Error_Absent,
    // Not an error message, but rather the number of error messages.
    Error_Count,
};

static inline const char *status_message(enum status status) {
    const char *messages[Error_Count] = {
        "Ok",
        "Not enough memory",
        "Invalid",
        "Permissions",
        "Already present",
        "Absent",
    };
    if (status >= Error_Count) {
        return "Bad error";
    }
    return messages[status];
}

#define bubble(condition, message) { \
        enum status error = condition; \
        if (error != Ok) { \
            log(Log_Warning, message); \
            return error; \
        } \
}

extern uint8_t __kernel_start;
extern uint8_t __kernel_end;

#define Kernel_Image_Size     ((uintptr_t)((byte *)&__kernel_end - \
                                           (byte *)&__kernel_start))
#define Kernel_Physical_Start ((void *)&__kernel_start)
#define Kernel_Physical_End   ((void *)&__kernel_end)
#define Kernel_Virtual_Start ((void *)&__kernel_start)
#define Kernel_Virtual_End   ((void *)&__kernel_end)

#define phys_to_virt(a) ((void *)a)

#define container_of(child, parent_type, parent_entry) \
    (child - ((parent_type)NULL)->parent_entry)
#define is_aligned(value, alignment) (value & (alignment - 1))
#define round_next(x, y) (((x) + (y - 1)) & ~(y - 1))
#define static_array_count(x) (sizeof(x) / sizeof(x)[0])

#define check_format(x, y) __attribute__((format(printf, x, y)))
#define checked __attribute__((warn_unused_result))
#define no_return __attribute__((no_return))
#define pack __attribute__((packed))
#define unused(x) x __attribute__((unused))
