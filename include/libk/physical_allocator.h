#ifndef PHYSICAL_ALLOCATOR_H
#define PHYSICAL_ALLOCATOR_H

typedef uint32_t page_frame_t;

#define PAGE_FRAME_CACHE_SIZE 32
#define PAGE_FRAME_MAP_SIZE (PHYS_MEMORY_SIZE/8/PAGE_SIZE)

// Linear search of page frame bitmap
page_frame_t alloc_frame_helper();

void free_frame(page_frame_t frame);

page_frame_t alloc_frame();

void use_frame(page_frame_t frame);

#endif
