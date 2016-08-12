#pragma once

#include <truth/memtypes.h>
#include <truth/types.h>

status_t checked init_higher_half(page_frame_t highest_address);

page_frame_t get_kernel_region(size_t pages, enum region_perms perms);

void put_kernel_region(page_frame_t region, size_t pages);
