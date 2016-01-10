#include <arch/x86/paging.h>

uint32_t *kernel_page_table_install() {
    // Certain very important things already exist in physical memory. They
    // need to be marked as present so that the allocator doesn't grab them by
    // accident.
    // After they are marked as present they can safely be mapped with
    // map_page.

    use_frame(0);

    // Mark all the pages the kernel sits on as in use
    for (page_frame_t i = KERNEL_START; i < KERNEL_END; i += PAGE_SIZE) {
        use_frame(i);
    }
    // Mark the kernel heap as in use
    use_frame(KHEAP_PHYS_ROOT);
    // Mark video memory as in use
    use_frame(VIDEO_MEMORY_BEGIN);
    // Mark the paging directory as in use
    use_frame(PAGE_DIRECTORY);

    uint32_t *page_dir = (uint32_t*)alloc_frame();
    kernel_pages = (uint32_t*)alloc_frame();
    memset(page_dir, 0, PAGE_SIZE);
    memset(kernel_pages, 0, PAGE_SIZE);

    // Fractal page mapping
    page_dir[PAGE_TABLE_SIZE-1] = GETADDRESS(page_dir) | PAGE_PRESENT;

    page_dir[PAGE_TABLE_SIZE-2] = GETADDRESS(kernel_pages) | PAGE_PRESENT;

    map_kernel_pages(page_dir);

    enable_paging(page_dir);
    return page_dir;
}

uint32_t create_new_page_dir(page_frame_t *cur_page_dir, void *link_loc,
        void *stack_loc, uint16_t permissions) {

    __asm__ volatile ("cli");
    disable_paging();

    page_frame_t *page_table = (page_frame_t*)alloc_frame();
    memset(page_table, 0, PAGE_SIZE);

    page_table[PAGE_TABLE_SIZE-1] = GETADDRESS(page_table) | PAGE_PRESENT;
    map_kernel_pages(page_table);
    map_page(page_table, (page_frame_t)link_loc, link_loc, permissions);
    map_page(page_table, (page_frame_t)stack_loc, stack_loc, permissions);

    klog("Loading new page table to make sure it is well formed\n");
    enable_paging(page_table);
    klog("Yep, it's valid. Reverting to old page table.\n");

    enable_paging(cur_page_dir);
    __asm__ volatile ("sti");

    return (uint32_t)page_table;
}

void *just_give_me_a_page(uint32_t *page_dir, uint16_t permissions) {
    uint32_t phys_addr = alloc_frame();

    // Walk the page directory in search of an available address
    for (size_t i = 0; i < PAGE_TABLE_SIZE-2; ++i) {
        if (page_dir[i] & PAGE_PRESENT) {
            uint32_t *page_entry = (uint32_t*)GETADDRESS(page_dir[i]);
            for (size_t j = 0; j < PAGE_TABLE_SIZE-1; ++j) {
                if (!(page_entry[j] & PAGE_PRESENT)) {
                    page_entry[j] = phys_addr | PAGE_PRESENT;
                    flush_tlb();
                    return (void*)((i << 22) | (j << 12));
                }
            }
        }

    }

    // Allocate a new page table and return its first address
    for (size_t i = 0; i < PAGE_TABLE_SIZE-2; ++i) {
        if (!(page_dir[i] & PAGE_PRESENT)) {
            uint32_t *page_table = (uint32_t*)alloc_frame();
            page_dir[i] = (uint32_t)page_table | PAGE_PRESENT;
            page_table[1] = phys_addr | permissions | PAGE_PRESENT;
            flush_tlb();
            return (void*)((1 << 22) | (0));
        }
    }

    // The page table is full. Fail by returning a special unaligned address
    return (void*)EPHYSMEMFULL;
}


void map_kernel_pages(uint32_t *page_dir) {
    // Map all those important bits
    for (page_frame_t i = KERNEL_START; i < KERNEL_END; i += PAGE_SIZE) {
        map_page(page_dir, i, (void*)i, 0);
    }
    map_page(page_dir, KHEAP_PHYS_ROOT, (void*)KHEAP_PHYS_ROOT, 0);

    // FIXME: Move to video memory driver
    map_page(page_dir, VIDEO_MEMORY_BEGIN, (void*)VIDEO_MEMORY_BEGIN, 0);
}

// TODO: refactor to use get_page
int map_page(uint32_t *page_dir, page_frame_t physical_page,
        void *virtual_address, uint16_t permissions) {

    if ((physical_page % PAGE_SIZE) != 0 ||
            ((uintptr_t)virtual_address % PAGE_SIZE) != 0) {
        return -1;
    }
    uint32_t *page_entry;
    uint32_t dir_index = HIGHINDEX(virtual_address);
    uint32_t entry_index = LOWINDEX(virtual_address);

    // If the relevant page directory entry is absent, allocate it.
    if ((page_dir[dir_index] & PAGE_PRESENT) == 0) {
        page_entry = (uint32_t*)alloc_frame();

        page_dir[dir_index] = GETADDRESS(page_entry) | permissions |
            PAGE_PRESENT;
    } else {
        page_entry = (uint32_t*)GETADDRESS(page_dir[dir_index]);
    }
    page_entry[entry_index] = GETADDRESS(virtual_address) | permissions |
        PAGE_PRESENT;
    return 0;
}


// TODO: refactor to use get_page
int unmap_page(page_frame_t *page_dir, void *virtual_address,
        bool should_free_frame) {

    uint32_t *page_entry;
    uint32_t dir_index = HIGHINDEX(virtual_address);
    uint32_t entry_index = LOWINDEX(virtual_address);

    // Original top level page table was not mapped. Return error.
    if ((page_dir[dir_index] & 1) == 0) {
        return -1;
    }

    page_entry = (uint32_t*)GETADDRESS(page_dir[dir_index]);
    // Clear present bit
    page_entry[entry_index] &= ~PAGE_PRESENT;

    if (should_free_frame) {
        page_frame_t physical_page = GETADDRESS(page_entry[entry_index]);
        free_frame(physical_page);
    }

    return 0;
}

uint32_t *get_page_entry(page_frame_t *page_dir, void *virtual_address) {
    uint32_t *page_entry;
    uint32_t dir_index = HIGHINDEX(virtual_address);

    if ((page_dir[dir_index] & 1) == 0) {
        return (uint32_t*)~0; // return all fs, an invalid value
    }

    page_entry = (uint32_t*)GETADDRESS(page_dir[dir_index]);
    return page_entry;
}

void free_table(uint32_t *page_dir) {
    // The last two entries of the page directory are reserved. The last one
    // points to the directory itself, and the second to last one points to
    // kernel space
    for (size_t i = 0; i < PAGE_TABLE_SIZE-2; ++i) {
        uint32_t *page_entry = (uint32_t*)GETADDRESS(page_dir[i]);
        for (size_t j = 0; j < PAGE_TABLE_SIZE; ++j) {
            if ((page_entry[j] & PAGE_PRESENT) == 1) {
                free_frame(page_entry[j]);
            }
        }
    }
    free_frame((page_frame_t)page_dir);
}
