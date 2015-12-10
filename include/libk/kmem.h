#ifndef KMEM_H
#define KMEM_H
#include <stdbool.h>
#include <stdint.h>
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


struct kheap_metadata {
    size_t size;
    struct kheap_metadata *next;
    bool is_free;
};


struct kheap_metadata *root;

struct kheap_metadata *kheap_init();

int kheap_extend();
void kheap_install(struct kheap_metadata *root, size_t initial_heap_size);
void *kmalloc(size_t bytes);
void *krealloc(void *ptr, size_t bytes);
void kfree(void *mem);
void kheap_defragment();
#endif
