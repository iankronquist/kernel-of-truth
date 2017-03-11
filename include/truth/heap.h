#pragma once

#include <truth/types.h>

void *kmalloc(size_t bytes);
void *krealloc(void *ptr, size_t bytes);
void *kcalloc(size_t count, size_t size);
void kfree(void *address);
enum status checked heap_init(void);
