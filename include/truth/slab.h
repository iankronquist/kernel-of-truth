#pragma once

#include <truth/memory.h>
#include <truth/types.h>

void slab_init(void);

void *slab_alloc(size_t bytes, enum memory_attributes attrs);
void *slab_alloc_phys(phys_addr *phys, enum memory_attributes attrs);
void slab_free(size_t count, void *address);

void *slab_alloc_locked(size_t bytes, enum memory_attributes attrs);
void slab_free_locked(size_t count, void *address);
