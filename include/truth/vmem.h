#pragma once

#include <stddef.h>

// A region of virtual memory.
struct virt_region;

// Permissions applied to virtual memory regions.
// They can be combined with logical or. The values are taken from the bits in
// x86 32 bit paging tables.
enum region_perms {
    REGION_WRITABLE      =  2,
    REGION_USER_MODE     =  4,
    REGION_WRITE_THROUGH =  8,
    REGION_UNCACHED      = 16,
};

// Get a region of virtual memory.
void *get_region(size_t pages, enum region_perms perms);

// Return a region of virtual memory to the kernel.
// It is up to the caller to ensure there are no leaks.
void put_region(void *region, size_t pages);

// Get a list of free virtual memory for a new address space.
struct virt_region *init_free_list(void);
