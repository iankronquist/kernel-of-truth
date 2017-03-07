#include <truth/types.h>
#include <truth/panic.h>

struct source_location {
    const char *file;
    uint32_t line;
    uint32_t column;
};

struct type_descriptor {
    uint16_t kind;
    uint16_t info;
    char name[];
};

struct type_mismatch_info {
    struct source_location location;
    struct type_descriptor *type;
    uintptr_t alignment;
    uint8_t type_check_kind;
};

struct out_of_bounds_info {
    struct source_location location;
    struct type_descriptor left_type;
    struct type_descriptor right_type;
};

static inline void log_location(struct source_location *location) {
    logf(Log_Error, "\tfile: %s\n\tline: %i\n\tcolumn: %i\n",
         location->file, location->line, location->column);
}

void __ubsan_handle_negate_overflow(struct source_location *location) {
    log(Log_Error, "Negation overflow");
    log_location(location);
    panic();
}

void __ubsan_handle_type_mismatch(struct type_mismatch_info *type_mismatch,
                                  uintptr_t pointer) {
    struct source_location *location = &type_mismatch->location;
    if (pointer == 0) {
        log(Log_Error, "Null pointer access");
    } else if (type_mismatch->alignment != 0 &&
               is_aligned(pointer, type_mismatch->alignment)) {
        log(Log_Error, "Unaligned memory access");
    } else {
        log(Log_Error, "Type mismatch");
        logf(Log_Error, "Type: %s\n", type_mismatch->type->name);
    }
    log_location(location);
    panic();
}

void __ubsan_handle_divrem_overflow(struct source_location *location,
                                    void *unused(left), void *unused(right)) {
    logf(Log_Error,
         "Division remainder overflow\n\tfile: %s\n\tline: %i\n\tcolumn: %i\n",
         location->file, location->line, location->column); panic();
    panic();
}

void __ubsan_handle_out_of_bounds(struct out_of_bounds_info *out_of_bounds) {
    log(Log_Error, "Out of bounds");
    log_location(&out_of_bounds->location);
    panic();
}

void __ubsan_handle_shift_out_of_bounds(struct out_of_bounds_info
                                        *out_of_bounds) {
    log(Log_Error, "Shift out of bounds");
    log_location(&out_of_bounds->location);
    panic();
}

void __ubsan_handle_mul_overflow(struct source_location *location,
                                 void *unused(left), void *unused(right)) {
    log(Log_Error, "Multiplication overflow");
    log_location(location);
    panic();
}

void __ubsan_handle_sub_overflow(struct source_location *location,
                                 void *unused(left), void *unused(right)) {
    log(Log_Error, "Subtraction overflow");
    log_location(location);
    panic();
}

void __ubsan_handle_load_invalid_value(struct source_location *location) {
    log(Log_Error, "Load invalid value");
    log_location(location);
    panic();
}
