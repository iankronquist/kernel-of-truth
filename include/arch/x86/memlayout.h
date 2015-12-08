#ifndef MEMLAYOUT_H
#define MEMLAYOUT_H

#include <stdint.h>

#define KERNEL_START ((uint32_t)&kernel_start)
#define KERNEL_END ((uint32_t)&kernel_end)
#define KERNEL_SIZE (KERNEL_START - KERNEL_END)

#define VIDEO_MEMORY_BEGIN 0xB8000
#define VIDEO_MEMORY_SIZE (80 * 24)

#define VIRTUAL_TO_PHYSICAL(addr) ((uint32_t)(addr) - KERNEL_START)
#define PHYSICAL_TO_VIRTUAL(addr) ((void *)(addr) + KERNEL_START)


#endif
