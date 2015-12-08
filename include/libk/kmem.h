#ifndef KMEM_H
#define KMEM_H
#include <stdbool.h>
#include <stdint.h>
#include <arch/x86/paging.h>
#include <libk/kabort.h>

#define KHEAP_PHYS_ROOT ((void*)0x100000)
//#define KHEAP_PHYS_END  ((void*)0xc1000000)
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
void kfree(void *mem);
void kheap_defragment();
#endif
