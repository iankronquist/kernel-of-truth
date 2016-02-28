#ifndef MEMLAYOUT_H
#define MEMLAYOUT_H

#include <stdint.h>

#define KB (1024)
#define MB (1024*1024)
#define GB (1024*1024*1024)

#define ACPI_BEGIN 0xa0000
#define ACPI_END   0xfffff

#define FRACTAL_MAP 0xffc00000

#define RING_0_DATA_SELECTOR 0x10

#define PHYS_MEMORY_SIZE (1 * GB)
#define PAGE_SIZE 4096

// These two variables are defined by the linker. They are located where you
// would expect based on the names.
extern const uint32_t kernel_start;
extern const uint32_t kernel_end;

#define KERNEL_END ((uint32_t)&kernel_end)
#define KERNEL_START ((uint32_t)&kernel_start)
#define KERNEL_END ((uint32_t)&kernel_end)
#define KERNEL_SIZE (KERNEL_START - KERNEL_END)

// Paging related
#define PAGE_ALIGN(x) (((uintptr_t)(x)) & ~0xfff)
#define NEXT_PAGE(x) (((uintptr_t)(x)+PAGE_SIZE) & ~0xfff)

// Heap related
#define KHEAP_PHYS_ROOT NEXT_PAGE(KERNEL_END)
#define KHEAP_PHYS_END ((void*)NEXT_PAGE(KHEAP_PHYS_ROOT))

// Also paging related
#define PAGE_DIRECTORY NEXT_PAGE(KERNEL_END)

// Video memory related
#define VIDEO_MEMORY_BEGIN 0xB8000
#define VIDEO_MEMORY_SIZE (80 * 24)

#endif
