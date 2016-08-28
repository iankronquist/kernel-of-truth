#pragma once

#include <truth/memtypes.h>
#include <truth/types.h>

/* Kernel higher half memory allocator.
 * Allocates memory from the "higher half", the pool of memory in the higher
 * part of the address space reserved for kernel use.
 */

// Initialize the higher half memory allocator.
status_t checked init_higher_half(void);

// Get a region of kernel only memory.
void *get_kernel_region(size_t pages, enum region_perms perms);

// Return a region of kernel only memory to the pool.
void put_kernel_region(void *region, size_t pages);

// Get a region of kernel only memory mapped to the provided page.
void *get_kernel_region_page(size_t pages, page_frame_t first_page,
        enum region_perms perms);
