#include <libk/physical_allocator.h>


static uint8_t page_frame_map[PAGE_FRAME_MAP_SIZE];
static page_frame_t frame_cache[PAGE_FRAME_CACHE_SIZE];

// Force allocation of frames on first call of alloc_frame
static uint8_t frame_count = PAGE_FRAME_CACHE_SIZE;

// Linear search of page frame bitmap
static void rebuild_frame_cache() {
    frame_count = 0;
    // The first page is reserved for mapping other pages
    for (size_t i = 0; i < PAGE_FRAME_MAP_SIZE; ++i) {
        if (i != 0xff) {
            for (size_t j = 0; j < 8; ++j) {
                if (!((page_frame_map[i] >> j) & 1)) {
                    frame_cache[frame_count] = (i * 8 + j) << 12;
                    frame_count++;
                    if (frame_count == PAGE_FRAME_CACHE_SIZE) {
                        frame_count = 0;
                        return;
                    }
                }
            }
        }
    }
    kputs("Cannot allocate any more pages.");
    kabort();
    return;
}

bool is_free_frame(page_frame_t frame) {
    frame >>= 12;
    return page_frame_map[BYTE_INDEX(frame)] & BIT_INDEX(frame);
}

void use_frame(page_frame_t frame) {
    frame >>= 12;
    page_frame_map[BYTE_INDEX(frame)] |= BIT_INDEX(frame);
    // Invalidate frame_cache because the frame we just marked as used may be
    // in it.
    frame_count = PAGE_FRAME_CACHE_SIZE;
}

void free_frame(page_frame_t frame) {
    frame >>= 12;
    page_frame_map[BYTE_INDEX(frame)] &= ~BIT_INDEX(frame);
}

page_frame_t alloc_frame() {
    page_frame_t new_frame;

    if (frame_count == PAGE_FRAME_CACHE_SIZE) {
        rebuild_frame_cache();
    }
    new_frame = frame_cache[frame_count];
    // Mark page as taken
    page_frame_map[BYTE_INDEX(new_frame>>12)] |= BIT_INDEX(new_frame>>12);
    frame_count++;
    kassert(new_frame % PAGE_SIZE == 0);
    return new_frame;
}
