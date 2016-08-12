#pragma once

#include <truth/memtypes.h>


status_t checked insert_region(void *addr, size_t size, struct region *root);
// Return a newly allocated region.
struct region *init_region(void *address, size_t size, struct region *next);
// Destroy a list of regions.
void destroy_free_list(struct region *vr);
// Find a free region of the given size.
void *find_region(size_t size, struct region *vr);
// Map a region into the current page process' page table.
void map_region(void *vr, size_t pages,  uint16_t perms);
// Unmap a region from the current page process' page table.
void unmap_region(void *vr, size_t pages);
