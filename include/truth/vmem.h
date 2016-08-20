#pragma once

#include <truth/memtypes.h>
#include <truth/types.h>

/* Per process virtual memory allocator.
 * Allocate memory from the current process's virtual memory.
 */

// Get a region of virtual memory.
void *get_region(size_t pages, enum region_perms perms);

// Return a region of virtual memory to the kernel.
// It is up to the caller to ensure there are no leaks.
void put_region(void *region, size_t pages);

// Get a list of free virtual memory for a new address space.
virt_region_t *init_free_list(void);
