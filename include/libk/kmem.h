#ifndef KMEM_H
#define KMEM_H
#include <stdbool.h>
#include <stdint.h>
#include <arch/x86/paging.h>
#include <libk/kabort.h>

#define KHEAP_PHYS_ROOT ((void*)0xc0000000)
#define KHEAP_PHYS_END  ((void*)0xc1000000)
#define KHEAP_END_SENTINEL NULL


struct kheap_metadata {
    size_t size;
    struct kheap_metadata *next;
    bool is_free;
};


struct kheap_metadata *root;

struct kheap_metadata *kheap_init();

void kheap_extend();
void kheap_install();
void *kmalloc(size_t bytes);
void kfree(void *mem);
void kheap_defragment();
#endif
