#pragma once

#include <truth/types.h>

enum status random_init(void);
void random_bytes(void *buf, size_t size);
void *random_address(bool kernel_space, uint64_t alignment);
