#pragma once

#include <truth/types.h>


void *boot_allocator(size_t pages);
void boot_allocator_init(uint64_t kilobytes);
