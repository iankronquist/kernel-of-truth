#pragma once

#include <truth/types.h>
#include <truth/memory.h>

// Memory_Writable implies NX. We need this for higher levels of the page table.
#define Memory_Just_Writable (1 << 1)
#define invalid_phys_addr 0xfff
enum status boot_map_page(const void *virtual_address, phys_addr phys_address, enum memory_attributes permissions, bool force);
