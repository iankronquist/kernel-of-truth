#pragma once

// Holds a region of virtual or physical memory.
struct region_head;

// A physical address.
typedef uintptr_t phys_addr_t;

// A region of virtual memory.
typedef struct region_head virt_region_t;

// A region of physical memory
typedef struct region_head phys_region_t;

// Permissions applied to virtual memory regions.
// They can be combined with logical or. The values are taken from the bits in
// x86 32 bit paging tables.
enum region_perms {
    REGION_WRITABLE      =  2,
    REGION_USER_MODE     =  4,
    REGION_WRITE_THROUGH =  8,
    REGION_UNCACHED      = 16,
};
