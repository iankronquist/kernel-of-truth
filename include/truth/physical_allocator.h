#pragma once

#include <external/multiboot.h>
#include <truth/types.h>

// Special unaligned address
#define invalid_phys_addr 0xfff

struct physical_region_vector;

void physical_allocator_init(struct multiboot_info *multiboot_tables);

phys_addr physical_alloc(void);

void physical_free(phys_addr address);
