#ifndef MEMLAYOUT_H
#define MEMLAYOUT_H

#include <stdint.h>

// 1 MB
#define KERNEL_START 0x1000000

// 4 KB
#define KERNEL_SIZE (4096*4)

#define VIDEO_MEMORY_BEGIN 0xB8000
#define VIDEO_MEMORY_SIZE (80 * 24)

#define VIRTUAL_TO_PHYSICAL(addr) ((uint32_t)(addr) - KERNEL_START)
#define PHYSICAL_TO_VIRTUAL(addr) ((void *)(addr) + KERNEL_START)


#endif
