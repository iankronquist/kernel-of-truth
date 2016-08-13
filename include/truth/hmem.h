#pragma once

#include <truth/memtypes.h>
#include <truth/types.h>

// Initialize the higher half memory allocator.
status_t checked init_higher_half(page_frame_t highest_address);

// Get a region of kernel only memory.
page_frame_t get_kernel_region(size_t pages, enum region_perms perms);

// Return a region of kernel only memory to the pool.
void put_kernel_region(page_frame_t region, size_t pages);
