#pragma once

#include <truth/types.h>

bool heap_tag_leak(int tag);
void *malloc(size_t bytes, int tag);
void *calloc(size_t count, size_t size, int tag);
void free(void *address, int tag);
enum status checked init_heap(void);
