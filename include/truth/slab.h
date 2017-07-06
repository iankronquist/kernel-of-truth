#pragma once

#include <truth/memory.h>
#include <truth/types.h>
#include <truth/region_vector.h>

void slab_init(void);

void *slab_alloc(size_t bytes, enum memory_attributes attrs);
void *slab_alloc_phys(phys_addr *phys, enum memory_attributes attrs);
void slab_free(size_t count, void *address);
void slab_free_virt(size_t count, void *address);

void *slab_alloc_locked(size_t bytes, enum memory_attributes attrs);
void slab_free_locked(size_t count, void *address);

void slab_free_helper(size_t bytes, void *address, struct region_vector *vect,
                      bool free_phys);
void *slab_alloc_helper(size_t bytes, phys_addr *phys,
                        enum memory_attributes page_attributes,
                        struct region_vector *vect);
void *slab_alloc_request_physical(phys_addr phys, size_t bytes, enum memory_attributes attrs);
void *slab_alloc_request_physical_random(phys_addr phys, size_t bytes, enum memory_attributes attrs);
size_t slab_get_usage(void);
