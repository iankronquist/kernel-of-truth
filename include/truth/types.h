#pragma once

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <truth/memory_sizes.h>

typedef uint8_t byte;
typedef char *string;

#define str(x) x
#define str_to_bytes(s) ((byte *)s)

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
};

#define bubble(condition, message) { \
        enum status error = condition; \
        if (error != Ok) { \
            log(message); \
        } \
}


extern byte __kernel_start;
extern byte __kernel_end;

#define kernel_image_size     ((uintptr_t)((byte *)&__kernel_end - \
                                           (byte *)&__kernel_start))
#define kernel_physical_start ((byte *)&__kernel_start)
#define kernel_physical_end   ((byte *)&__kernel_end)

#define container_of(child, parent_type, parent_entry) \
    (child - ((parent_type)NULL)->parent_entry)
#define is_aligned(value, alignment) (value & (alignment - 1))

// #define format(x, y) __attribute__((printf(x, y)))
#define checked __attribute__((warn_unused_result))
#define no_return __attribute__((no_return))
#define pack __attribute__((packed))
#define unused(x) x __attribute__((unused))
