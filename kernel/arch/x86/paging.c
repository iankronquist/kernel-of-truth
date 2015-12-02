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
*/


void kernel_page_table_install() {

    memset(page_frame_map, 0, PAGE_FRAME_MAP_SIZE);

    // Mark the kernel as mapped
    page_frame_map[KERNEL_START/8] = 1 << (KERNEL_START % 8);
    page_frame_t phys_page_dir = kalloc_frame();
    page_frame_t phys_page_entries = kalloc_frame();
    // Zero page table
    memset((void*)phys_page_dir, 0, PAGE_SIZE);
    memset((void*)phys_page_entries, 0, PAGE_SIZE);

    struct page_dir_entry *dirs = phys_page_dir;
    struct page_entry *entries = phys_page_entries;;
    dirs[TOP10(KERNEL_START)].present = 1;
    dirs[TOP10(KERNEL_START)].address = phys_page_entries >> 12;

    entries[BOTTOM10(KERNEL_START)].present = 1;
    entries[BOTTOM10(KERNEL_START)].address = KHEAP_PHYS_ROOT;

    entries[BOTTOM10(KERNEL_START+PAGE_SIZE)].present = 1;
    entries[BOTTOM10(KERNEL_START+PAGE_SIZE)].address = PAGE_ALIGN(KHEAP_PHYS_ROOT+PAGE_SIZE);

    entries[BOTTOM10(KERNEL_START+2*PAGE_SIZE)].present = 1;
    entries[BOTTOM10(KERNEL_START+2*PAGE_SIZE)].address = PAGE_ALIG(NKHEAP_PHYS_ROOT+2*PAGE_SIZE);

    entries[BOTTOM10(KERNEL_START+3*PAGE_SIZE)].present = 1;
    entries[BOTTOM10(KERNEL_START+3*PAGE_SIZE)].address = PAGE_ALIGN(KHEAP_PHYS_ROOT+3*PAGE_SIZE);

    int foo;
    kprint_int("item: ", KERNEL_START);
    kprint_int("item: ", (int*)&foo);
    /*
    entries[(phys_page_dir)].present = 1;
    entries[(phys_page_dir)].address = 0x5;

    entries[phys_page_entries].present = 1;
    entries[phys_page_entries].address = 0x9;
    */

    enable_paging(phys_page_dir);
}
