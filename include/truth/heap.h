#pragma once

#include <truth/types.h>

void *malloc(size_t bytes, int tag);
void *calloc(size_t count, size_t size, int tag);
void free(void *address, int tag);
enum status checked init_heap(void);
