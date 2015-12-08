#include <arch/x86/paging.h>

static uint8_t page_frame_map[PAGE_FRAME_MAP_SIZE];
static page_frame_t frame_cache[PAGE_FRAME_CACHE_SIZE];
static page_frame_t first_frame = 0;


void init_page(page_frame_t phys_addr, uint32_t virt_addr, uint32_t *dirs);
struct page_entry *get_page(page_frame_t phys_addr, struct page_dir_entry *dir);

/*
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
    kassert(new_frame % PAGE_SIZE == 0);
    kprintf("New frame address: %p\n", new_frame);
    return new_frame;
}
*/

#define PAGE_ALIGN(x) (((uintptr_t)(x)) & ~0xfff)
#define NEXT_PAGE(x) (((uintptr_t)(x)+PAGE_SIZE) & ~0xfff)
#define TOP20(x) ((uintptr_t)(x) & 0xfffff000)
#define TOP10(x) ((uintptr_t)(x) & 0xffc00000)
#define MID10(x) ((uintptr_t)(x) & 0x003ff000)
#define LOW10(x) ((uintptr_t)(x) & 0x000003ff)
#define PAGE_TABLE_SIZE 1024
#define PAGE_DIRECTORY NEXT_PAGE(&kernel_end)


void kernel_page_table_install() {
    uint32_t *page_dir = PAGE_DIRECTORY;
    uint32_t *cur_page_entry = PAGE_DIRECTORY + PAGE_SIZE;
    for (uint32_t table_i = 0; table_i < PAGE_TABLE_SIZE; table_i++) {
        page_dir[table_i] = TOP20(cur_page_entry) | 1;
        for (uint32_t entry_i = 0; entry_i < PAGE_TABLE_SIZE; entry_i++) {
            cur_page_entry[entry_i] = LOW10(table_i) << 22 | LOW10(entry_i) << 12 | 1;
        }
        cur_page_entry += PAGE_SIZE;
    }
    enable_paging(page_dir);
}

void init_page(page_frame_t phys_addr, uint32_t virt_addr, uint32_t *dirs) {
    static uint32_t last_page_frame = (uint32_t)KERNEL_END + 2 * PAGE_SIZE;
    uint32_t top10 = phys_addr >> 22;
    uint32_t bottom10 = (phys_addr >> 12) & 0x3ff;
    page_frame_t *page_frame;
    // If the page table is not present, allocate it
    if ((dirs[top10] & 0x1) == 0) {
        page_frame = (uint32_t*)last_page_frame;
        last_page_frame += PAGE_SIZE;
        // Set pointer to page table
        dirs[top10] |= (uint32_t)page_frame & 0xffffc000;
        // Set present bit
        dirs[top10] |= 0x1;
    }
    page_frame = (page_frame_t*)(dirs[top10] & 0xffffc000);

    // Set pointer to the virt_addr
    page_frame[bottom10] = virt_addr & 0xffffc000;
    // Set present bit
    page_frame[bottom10] |= 0x1;
    //kprintf("Page frame: %p\n", page_frame);
    kprintf("Page frame bt: %p\n", page_frame[bottom10]);
    //kprintf("Page dir: %p\n", dirs);
    kprintf("Page dir tt: %p\n", dirs[top10]);
}
