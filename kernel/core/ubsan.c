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

void __ubsan_handle_negate_overflow(struct source_location *location) {
    logf("Negation overflow\n\tfile: %s\n\tline: %i\n\tcolumn: %i\n",
         location->file, location->line, location->column);
    panic();
}

void __ubsan_handle_type_mismatch(struct type_mismatch_info *type_mismatch,
                                  uintptr_t pointer) {
    char *violation;
    struct source_location *location = &type_mismatch->location;
    if (pointer == 0) {
        violation = "Null pointer access";
    } else if (type_mismatch->alignment != 0 &&
               is_aligned(pointer, type_mismatch->alignment)) {
        violation = "Unaligned memory access";
    } else {
        violation = "Type mismatch";
        logf("Type: %s\n", type_mismatch->type->name);
    }
    logf("%s\n\tfile: %s\n\tline: %i\n\tcolumn: %i\n", violation,
         location->file, location->line, location->column);
    panic();
}

void __ubsan_handle_divrem_overflow(struct source_location *location,
                                    void *unused(left), void *unused(right)) {
    logf("Division remainder overflow\n\tfile: %s\n\tline: %i\n\tcolumn: %i\n",
         location->file, location->line, location->column); panic();
    panic();
}

void __ubsan_handle_out_of_bounds(struct out_of_bounds_info *out_of_bounds) {
    struct source_location *location = &out_of_bounds->location;
    logf("Out of bounds\n\tfile: %s\n\tline: %i\n\tcolumn: %i\n",
         location->file, location->line, location->column);
    panic();
}

void __ubsan_handle_shift_out_of_bounds(struct out_of_bounds_info
                                        *out_of_bounds) {
    struct source_location *location = &out_of_bounds->location;
    logf("Shift out of bounds\n\tfile: %s\n\tline: %i\n\tcolumn: %i\n",
         location->file, location->line, location->column);
    panic();
}
