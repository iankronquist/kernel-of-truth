#pragma once

#include <truth/types.h>

void random_bytes(void *buf, size_t size);
void *random_address(bool kernel_space, uint64_t alignment);
