#ifndef MEMLAYOUT_H
#define MEMLAYOUT_H

#include <stdint.h>

#define KB 1024
#define MB (1024*1024)
#define GB (1024*1024*1024)

#define FRACTAL_MAP 0xffc00000


// The address of the start of the kernel. Defined by the linker.
extern uint32_t kernel_start;
// The address of the end of the kernel. Defined by the linker.
extern uint32_t kernel_end;
// The address of the start of the heap used for bootstrapping the rest of the
// kernel.
extern uint32_t bootstrap_heap;

#define KERNEL_END ((uint32_t)&kernel_end)
#define KERNEL_START ((uint32_t)&kernel_start)
#define KERNEL_SIZE (KERNEL_START - KERNEL_END)

#define BOOTSTRAP_HEAP_SIZE (16*KB)
#define BOOTSTRAP_HEAP_START &bootstrap_heap

// Paging related
#define PAGE_ALIGN(x) (void*)(((uintptr_t)(x)) & ~0xfff)
#define NEXT_PAGE(x) (((uintptr_t)(x)+PAGE_SIZE) & ~0xfff)
#define PAGE_SIZE 4096

// A physical address.
typedef uint32_t page_frame_t;

// Video memory related
#define VIDEO_MEMORY_BEGIN 0xc03ff000
#define VIDEO_MEMORY_SIZE (80 * 24)
#define VGA_PHYS_BEGIN 0xa0000
#define VGA_PHYS_END   0xbffff

#endif
