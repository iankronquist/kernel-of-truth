#pragma once

#include <truth/memtypes.h>

status_t checked insert_region(void *addr, uint64_t size,
        struct region_head *head);
// Return a newly allocated region.
struct region_head *init_region_list(void);
// Destroy a list of regions.
void destroy_free_list(struct region_head *vr);
// Find a free region of the given size.
void *find_region(size_t size, struct region_head *vr);
// Map a region into the current page process' page table.
void map_region(void *vr, size_t pages,  uint16_t perms);
// Unmap a region from the current page process' page table.
void unmap_region(void *vr, size_t pages);
