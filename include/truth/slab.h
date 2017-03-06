#pragma once

#include <truth/memory.h>
#include <truth/types.h>

void init_slab(void);

void *slab_alloc(size_t count, enum page_size size,
                 enum memory_attributes attrs);
void slab_free(size_t count, enum page_size size, void *address);
