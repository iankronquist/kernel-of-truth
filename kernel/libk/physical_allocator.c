#include <libk/physical_allocator.h>

static uint8_t page_frame_map[PAGE_FRAME_MAP_SIZE];
static page_frame_t frame_cache[PAGE_FRAME_CACHE_SIZE];
static page_frame_t first_frame = 0;

// Force allocation of frames on first call of kalloc_frame
static uint8_t frame_count = PAGE_FRAME_CACHE_SIZE;

// Linear search of page frame bitmap
page_frame_t alloc_frame_helper() {
    for (size_t i = 0; i < PAGE_FRAME_MAP_SIZE; ++i) {
        if (i != 0xff) {
            for (size_t j = 0; j < 8; ++j) {
                if (!((page_frame_map[i] >> j) & 1)) {
                    page_frame_map[i] |= 1 << j;
                    return first_frame + (i * 8 + j) * PAGE_SIZE;
                }
            }
        }
    }
    kputs("Cannot allocate any more pages.");
    kabort();
    return 0;
}

void use_frame(page_frame_t frame) {
    frame = (frame - first_frame)/PAGE_SIZE;
    page_frame_map[frame/8] |= 1 << (frame % 8);

    // Invalidate frame_cache because the frame we just marked as used may be
    // in it.
    frame_count = PAGE_FRAME_CACHE_SIZE;
}

void free_frame(page_frame_t frame) {
    frame = (frame - first_frame)/PAGE_SIZE;
    page_frame_map[frame/8] &= ~(1 << (frame % 8));
}

page_frame_t kalloc_frame() {
    page_frame_t new_frame;

    if (frame_count == PAGE_FRAME_CACHE_SIZE) {
        for (size_t i = 0; i < PAGE_FRAME_CACHE_SIZE; ++i) {
            frame_cache[i] = kalloc_frame_helper();
        }
        frame_count = 0;
    }
    new_frame = frame_cache[frame_count];
    frame_count++;
    kassert(new_frame % PAGE_SIZE == 0);
    kprintf("New frame address: %p\n", new_frame);
    return new_frame;
}
