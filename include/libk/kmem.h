#ifndef KMEM_H
#define KMEM_H
#include <stdbool.h>
#include <stdint.h>
#include <libk/kmemcpy.h>
#include <libk/kabort.h>
#include <libk/kassert.h>

#ifdef ARCH_X86
#include <arch/x86/memlayout.h>
#include <arch/x86/paging.h>
#endif

#ifdef ARCH_USERLAND
#include "tests/memlayout.h"
#endif


// KHEAP_PHYS_ROOT is defined in memlayout.h because it is architecture
// specific.
#define KHEAP_END_SENTINEL (NULL)

#define KHEAP_BLOCK_SLOP 32

// Keep allocations on the kernel heap in a linked list.
// This information is squirreled away before each allocation on the heap.
struct kheap_metadata {
    size_t size;
    struct kheap_metadata *next;
    bool is_free;
};

// Extend heap by page sized increments
int kheap_extend(size_t bytes);

// Install the kernel heap
// Place the heap at the given root, with an initial size.
void kheap_install(struct kheap_metadata *root, size_t initial_heap_size);

// Similar to malloc.
void *kmalloc(size_t bytes);

// Similar to realloc.
void *krealloc(void *ptr, size_t bytes);

// Similar to free.
void kfree(void *mem);

// Defragment the kernel heap.
void kheap_defragment(void);

#endif
