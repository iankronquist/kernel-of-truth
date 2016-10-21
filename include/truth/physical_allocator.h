#pragma once

#include <external/multiboot.h>
#include <truth/types.h>

// Special unaligned address
#define invalid_phys_addr 0xfff

struct physical_region_vector;

void init_physical_allocator(struct multiboot_info *multiboot_tables);

phys_addr physical_alloc(size_t pages);

void physical_free(phys_addr address, size_t pages);
