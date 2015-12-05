#include <arch/x86/paging.h>

static uint8_t page_frame_map[PAGE_FRAME_MAP_SIZE];
static page_frame_t frame_cache[PAGE_FRAME_CACHE_SIZE];
static page_frame_t first_frame = &kernel_end;


struct page_entry *init_page(page_frame_t phys_addr, void* virt_addr,
        struct page_dir_entry *dir, bool super_user, bool rw);
struct page_entry *get_page(page_frame_t phys_addr, struct page_dir_entry *dir);

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
    static uint8_t frame_count = PAGE_FRAME_CACHE_SIZE - 1;
    page_frame_t new_frame;

    if (frame_count == PAGE_FRAME_CACHE_SIZE - 1) {
        for (size_t i = 0; i < PAGE_FRAME_CACHE_SIZE; ++i) {
            frame_cache[i] = kalloc_frame_helper();
        }
        frame_count = 0;
    }
    new_frame = frame_cache[frame_count];
    frame_count++;
    return new_frame;
}


void kernel_page_table_install() {
    memset(&page_frame_map, 0 , PAGE_FRAME_MAP_SIZE);
    page_frame_t dirs = kalloc_frame();
    memset(dirs, 0, PAGE_SIZE);
    // Get all the kernel frames and identity map them
    for (page_frame_t phys_addr = KERNEL_START;
            phys_addr < (page_frame_t)&kernel_end; phys_addr += PAGE_SIZE) {
        page_frame_map[phys_addr/8] |= 1 << (phys_addr % 8);
        init_page(phys_addr, (void*)phys_addr, dirs, 0, 0);
    }
    enable_paging((struct page_dir_entry*)dirs);
}

struct page_entry *init_page(page_frame_t phys_addr, void* virt_addr,
        struct page_dir_entry *dir, bool super_user, bool rw) {
    uint32_t table_index = TOP10(phys_addr);
    struct page_entry *pages = dir[table_index].address;
    struct page_entry page = pages[BOTTOM10(phys_addr)];

    dir[table_index].present = 1;
    page.address = (uint32_t) virt_addr;
    page.present = 1;
    page.rw = rw;
    page.super_user = super_user;
    return &page;
}
