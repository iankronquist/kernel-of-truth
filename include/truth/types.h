#pragma once

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

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
    perm_read = 1,
    perm_write = 2,
    perm_execute = 3,
};

enum status {
    Ok,
    Error_No_Memory,
    Error_Invalid,
    Error_Permissions,
    Error_Present,
};

#define bubble(condition, message) { \
        enum status error = condition; \
        if (error != Ok) { \
            log(message); \
        } \
}

#define container_of(child, parent_type, parent_entry) \
    (child - ((parent_type)NULL)->parent_entry)

// #define format(x, y) __attribute__((printf(x, y)))
#define checked __attribute__((warn_unused_result))
#define no_return __attribute__((no_return))
#define pack __attribute__((packed))
#define unused __attribute__((unused))
