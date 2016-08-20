#pragma once

#include <truth/types.h>

/* Kernel heap allocator.
 * Allocate memory from the kernel heap.
 */

// Extend heap by page sized increments
int kheap_extend(size_t bytes);

// Install the kernel heap
// Place the heap at the given root, with an initial size.
void kheap_install(void *root, size_t initial_heap_size);

// Similar to malloc.
void *kmalloc(size_t bytes);

// Similar to realloc.
void *krealloc(void *ptr, size_t bytes);

// Similar to free.
void kfree(void *mem);

// Defragment the kernel heap.
void kheap_defragment(void);
