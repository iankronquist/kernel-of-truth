#ifndef MEMLAYOUT_H
#define MEMLAYOUT_H

#include <truth/types.h>

#define KB 1024
#define MB (1024*1024)
#define GB (1024*1024*1024)

// FIXME: Move elsewhere
#define FRACTAL_MAP 0xffc00000


// The address of the start of the kernel. Defined by the linker.
extern int kernel_start;
// The address of the end of the kernel. Defined by the linker.
extern int kernel_end;

#define KERNEL_END ((uint32_t)&kernel_end)
#define KERNEL_START ((uint32_t)&kernel_start)
#define KERNEL_SIZE (KERNEL_START - KERNEL_END)

// Paging related
#define PAGE_ALIGN(x) (((uintptr_t)(x)) & ~0xfff)
#define NEXT_PAGE(x) (((uintptr_t)(x)+PAGE_SIZE) & ~0xfff)
#define PAGE_DIRECTORY NEXT_PAGE(KERNEL_END)
#define PAGE_SIZE 4096

// Heap related
#define KHEAP_PHYS_ROOT NEXT_PAGE(KERNEL_END)
#define KHEAP_PHYS_END ((void*)NEXT_PAGE(KHEAP_PHYS_ROOT))

// FIXME: Move elsewhere
// Video memory related
#define VIDEO_MEMORY_BEGIN 0xB8000
#define VIDEO_MEMORY_SIZE (80 * 24)
#define VGA_BEGIN 0xa0000
#define VGA_END   0xbffff

#endif
