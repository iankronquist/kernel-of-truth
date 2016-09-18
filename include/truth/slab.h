#pragma once

#include <truth/memory_sizes.h>
// FIXME move enum page_attributes into memory_sizes and rename both the enum
// and the header.
#include <arch/x64/paging.h>

enum slab_type {
    slab_small = SMALL_PAGE,
    slab_medium = MEDIUM_PAGE,
    slab_large = LARGE_PAGE,
};

enum slab_attributes {
    slab_kernel_memory,
    slab_user_memory,
};

void init_slab(void);

void*slab_alloc(size_t count, enum slab_type type,
                enum slab_attributes attrs,
                enum page_attributes page_attributes);
void slab_free(size_t count, enum slab_type type, void *address);
