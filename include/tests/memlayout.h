#ifndef MEMLAYOUT_H
#define MEMLAYOUT_H

#include <stdint.h>
#include <stddef.h>

typedef uint32_t page_frame_t;

// Make the number of entries 1024 for the physical_allocator test
#define PHYS_MEMORY_SIZE 8*1024*4096
#define PAGE_SIZE 4096
uintptr_t dependencies_suck;
#define PLAYGROUND_SIZE (PAGE_SIZE * 5)
#define KHEAP_PHYS_END (void*)(dependencies_suck+PLAYGROUND_SIZE)
#define KERNEL_END 0

#endif
