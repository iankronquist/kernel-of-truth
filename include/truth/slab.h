#pragma once

#include <truth/types.h>

enum status checked init_slab(void);

void *slab_alloc(size_t size, enum memory_attributes attrs, int tag);
void slab_free(void *address, int tag);
bool slab_tag_leak(int tag);
