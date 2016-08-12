#pragma once

#include <truth/types.h>

#include <contrib/multiboot.h>

// Initialize the physical memory allocator using the multiboot tables
status_t checked init_phys_allocator(struct multiboot_info *mb,
        page_frame_t *highest_address);

// Find an available range of physical memory.
page_frame_t get_phys_region(size_t pages);

// Return a range of physical memory to the pool.
void put_phys_region(page_frame_t region, size_t pages);

// Get a single page.
page_frame_t alloc_frame(void);

// Return a single page to the pool.
void free_frame(page_frame_t frame);
