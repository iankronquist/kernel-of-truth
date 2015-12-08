#ifndef MEMLAYOUT_H
#define MEMLAYOUT_H

#include <stdint.h>

// These two variables are defined by the linker. They are located where you
// would expect based on the names.
extern uint32_t kernel_start;
extern uint32_t kernel_end;

#define KERNEL_END ((uint32_t)&kernel_end)
#define KERNEL_START ((uint32_t)&kernel_start)
#define KERNEL_END ((uint32_t)&kernel_end)
#define KERNEL_SIZE (KERNEL_START - KERNEL_END)

// Paging related
#define PAGE_ALIGN(x) (((uintptr_t)(x)) & ~0xfff)
#define NEXT_PAGE(x) (((uintptr_t)(x)+PAGE_SIZE) & ~0xfff)
#define PAGE_DIRECTORY NEXT_PAGE(KERNEL_END)

// Heap related
#define KHEAP_PHYS_ROOT ((void*)0x100000)
#define KHEAP_PHYS_END ((void*)NEXT_PAGE(KHEAP_PHYS_ROOT))

// Video memory related
#define VIDEO_MEMORY_BEGIN 0xB8000
#define VIDEO_MEMORY_SIZE (80 * 24)

#endif
