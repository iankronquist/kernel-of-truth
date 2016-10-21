#pragma once

#include <truth/types.h>

void init_slab(void);

void *slab_alloc(size_t size, enum memory_attributes attrs, int tag);
void slab_free(size_t size, void *address, int tag);
bool slab_tag_leak(int tag);
