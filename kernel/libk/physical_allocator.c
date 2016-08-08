#include <truth/physical_allocator.h>
#include <contrib/multiboot.h>

#include <truth/kabort.h>
#include <truth/kassert.h>
#include <truth/kmem.h>
#include <truth/kputs.h>
#include <truth/lock.h>

#include <truth/private/memlayout.h>

#ifdef ARCH_USERLAND
#include "tests/memlayout.h"
#endif

#define BIT_INDEX(x) (1 << ((x) % 8))
#define BYTE_INDEX(x) ((x)/8)
#define PAGE_FRAME_CACHE_SIZE 32
#define PAGE_FRAME_MAP_SIZE(x) (x/8/PAGE_SIZE)

// A lock to prevent multiple process from modifying the page frame map at
// once.
static spinlock_t big_lock = SPINLOCK_INIT;
// The size of the page frame map.
static size_t page_frame_map_size;
// A heap allocated bitmap representing physical memory. Each bit represents a
// page in memory. It can be 0, for free, or 1, for in use.
static uint8_t *page_frame_map;
// Iterating over the bitmap in search of free pages is slow -- keep a cache of
// them around to amortize the linear time search cost.
static page_frame_t frame_cache[PAGE_FRAME_CACHE_SIZE];

// Force allocation of frames on first call of alloc_frame
static uint8_t frame_count = PAGE_FRAME_CACHE_SIZE;

void physical_allocator_init(size_t phys_memory_size) {
    page_frame_map_size = PAGE_FRAME_MAP_SIZE(phys_memory_size);
    page_frame_map = kmalloc(page_frame_map_size);
}

// Linear search of page frame bitmap
static void rebuild_frame_cache() {
    acquire_spinlock(&big_lock);
    frame_count = 0;
    // The first page is reserved for mapping other pages
    for (size_t i = 0; i < page_frame_map_size; ++i) {
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
    release_spinlock(&big_lock);
    return;
}

bool is_free_frame(page_frame_t frame) {
    frame >>= 12;
    return page_frame_map[BYTE_INDEX(frame)] & BIT_INDEX(frame);
}

void use_range(page_frame_t begin, page_frame_t end) {
    acquire_spinlock(&big_lock);
    kassert(begin < end);
    page_frame_map[BYTE_INDEX(begin)] = (BIT_INDEX(begin)-1) |
        BIT_INDEX(begin);
    page_frame_map[BYTE_INDEX(end)] = ~(BIT_INDEX(end)-1);
    for (size_t i = BYTE_INDEX(begin)+1; i < BYTE_INDEX(end)-1; ++i) {
        page_frame_map[i] = ~0;
    }
    release_spinlock(&big_lock);
}

void use_frame(page_frame_t frame) {
    acquire_spinlock(&big_lock);
    frame >>= 12;
    page_frame_map[BYTE_INDEX(frame)] |= BIT_INDEX(frame);
    // Invalidate frame_cache because the frame we just marked as used may be
    // in it.
    frame_count = PAGE_FRAME_CACHE_SIZE;
    release_spinlock(&big_lock);
}

void free_frame(page_frame_t frame) {
    acquire_spinlock(&big_lock);
    frame >>= 12;
    page_frame_map[BYTE_INDEX(frame)] &= ~BIT_INDEX(frame);
    release_spinlock(&big_lock);
}

page_frame_t alloc_frame() {
    acquire_spinlock(&big_lock);
    page_frame_t new_frame;

    if (frame_count == PAGE_FRAME_CACHE_SIZE) {
        rebuild_frame_cache();
    }
    new_frame = frame_cache[frame_count];
    // Mark page as taken
    page_frame_map[BYTE_INDEX(new_frame>>12)] |= BIT_INDEX(new_frame>>12);
    frame_count++;
    kassert(new_frame % PAGE_SIZE == 0);
    release_spinlock(&big_lock);
    return new_frame;
}
