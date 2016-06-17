#ifndef PHYSICAL_ALLOCATOR_H
#define PHYSICAL_ALLOCATOR_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <contrib/multiboot.h>

#include <libk/kabort.h>
#include <libk/kassert.h>
#include <libk/kmem.h>
#include <libk/kputs.h>

#ifdef ARCH_X86
#include <arch/x86/memlayout.h>
#endif

#ifdef ARCH_USERLAND
#include "tests/memlayout.h"
#endif


#define BIT_INDEX(x) (1 << ((x) % 8))
#define BYTE_INDEX(x) ((x)/8)
#define PAGE_FRAME_CACHE_SIZE 32
#define PAGE_FRAME_MAP_SIZE(x) (x/8/PAGE_SIZE)

void physical_allocator_init(size_t phys_memory_size);

// Linear search of page frame bitmap
page_frame_t alloc_frame_helper();

void free_frame(page_frame_t frame);

page_frame_t alloc_frame();

void use_frame(page_frame_t frame);

bool is_free_frame(page_frame_t frame);

void use_range(page_frame_t begin, page_frame_t end);

#endif
