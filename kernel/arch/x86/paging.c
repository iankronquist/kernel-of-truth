#include <arch/x86/paging.h>

static uint8_t page_frame_map[PAGE_FRAME_MAP_SIZE];
static page_frame_t frame_cache[PAGE_FRAME_CACHE_SIZE];
static page_frame_t first_frame = &kernel_end;

// Linear search of page frame bitmap
page_frame_t kalloc_frame_helper() {
    for (size_t i = 0; i < PAGE_FRAME_MAP_SIZE; ++i) {
        if (i != 0xff) {
            for (size_t j = 0; j < 8; ++j) {
                if (!((page_frame_map[i] >> j) & 1)) {
                    page_frame_map[i] |= 1 << j;
                    return first_frame + (i * 8 + j) * 0x1000;
                }
            }
        }
    }
    kputs("Cannot allocate any more pages.");
    kabort();
    return 0;
}

void kfree_frame(page_frame_t frame) {
    frame = (frame - first_frame)/0x1000;
    page_frame_map[frame/8] &= ~(frame % 8);
}

page_frame_t kalloc_frame() {
    // Force allocation of frames on first call
    static uint8_t num_frames = PAGE_FRAME_CACHE_SIZE - 1;
    page_frame_t new_frame;

    if (num_frames == PAGE_FRAME_CACHE_SIZE - 1) {
        for (size_t i = 0; i < PAGE_FRAME_CACHE_SIZE; ++i) {
            frame_cache[i] = kalloc_frame_helper();
        }
        num_frames = 0;
    }
    new_frame = frame_cache[num_frames];
    num_frames++;
    return new_frame;
}

/*
// 4kb of memory aligned on a 4kb boundary.
//alignas(PAGE_SIZE) struct page_dir_entry page_table[1024] = {{0}};
struct page_dir_entry page_table[1024] = {{0}};

struct page_dir_entry *alloc_page_dir_entry(bool rw, bool super_user,
        bool write_cache, bool disable_cache) {
    struct page_entry *pages = (struct page_dir_entry*)kalloc_frame();
    // Set 'present' bits too 0
    memset(pages, 0, PAGE_SIZE);
    for (size_t i = 0; i < 1024; ++i) {
        if (page_table[i].present == 0) {
            // Zero the entry in case the dirty bit, etc. are set.
            memset(&page_table[i], 0, sizeof(struct page_dir_entry));
            // Set the entry as present
            page_table[i].present = 1;
            // Other settings
            page_table[i].rw = rw;
            page_table[i].super_user = super_user;
            page_table[i].write_cache = write_cache;
            page_table[i].disable_cache = disable_cache;
            // Take top 10 bits
            page_table[i].address = (uint32_t)pages >> 12;
            return &page_table[i];
        }
    }
    // Out of memory!
    return NULL;
}

void free_page_dir_entry(struct page_dir_entry *dir) {
    struct page_entry *pages = (struct page_entry*)((uint32_t)(dir->address) <<
            12);
    kfree_frame(pages);
    memset(dir, 0, sizeof(struct page_dir_entry));
}

void *map_page(void *virtual, bool rw, bool super_user, bool write_cache,
        bool disable_cache) {
    void *physical_address = kalloc_frame();
    uint32_t top10 = ((uint32_t)virtual >> 12) >> 10;
    uint32_t bottom10 = ((uint32_t)virtual >> 12) & 0x3f;
    struct page_entry *entries = (struct page_entry*)page_table[top10].address;
    // Someone is already using this virtual address
    if (entries[bottom10].present) {
        return NULL;
    }
    // Zero dirty bits, etc.
    memset(&entries[bottom10], 0, sizeof(struct page_entry));
    entries[bottom10].present = 1;
    entries[bottom10].rw = rw;
    entries[bottom10].super_user = super_user;
    entries[bottom10].write_cache = write_cache;
    entries[bottom10].disable_cache = disable_cache;
    entries[bottom10].address = (uint32_t)physical_address >> 12;
    return virtual;
}

void unmap_page(void *virtual) {
    uint32_t top10 = ((uint32_t)virtual >> 12) >> 10;
    uint32_t bottom10 = ((uint32_t)virtual >> 12) & 0x3f;
    struct page_entry *entries = (struct page_entry*)page_table[top10].address;
    // present bit and others.
    memset(&entries[bottom10], 0, sizeof(struct page_dir_entry));
    kfree_frame(entries[bottom10].address);
}

void map_page(struct page_entry *page, uint32_t phys_address) {
    page->address = TOP20(phys_address);
    page->rw = 1;
    page->present = 1;
}

*/

void kernel_page_table_install() {
    // Place pages right after the kernel
    struct page_dir_entry *dirs = (void*)&kernel_end;
    struct page_entry *entries = (void*)&kernel_end + PAGE_TABLE_SIZE *
        sizeof(struct page_dir_entry);
    for (size_t i = 0; i < PAGE_TABLE_SIZE; ++i) {
        dirs[i].address = TOP20(&kernel_end + i * PAGE_SIZE);
        dirs[i].present = 1;
        dirs[i].rw = 1;
        dirs[i].super_user = 0;
        for (size_t j = 0; j < PAGE_TABLE_SIZE; ++j) {
            entries[i].address = MIDDLE20(&kernel_end + j * PAGE_SIZE);
            //entries[j].address = TOP20(i * PAGE_TABLE_SIZE + j) *
                sizeof(struct page_entry);
            entries[j].present = 1;
            entries[j].rw = 1;
            entries[j].super_user = 0;
        }
    }


    enable_paging(dirs);
}

