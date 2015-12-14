#include <arch/x86/paging.h>

uint32_t *kernel_page_table_install() {
    // Certain very important things already exist in physical memory. They
    // need to be marked as present so that the allocator doesn't grab them by
    // accident.
    // After they are marked as present they can safely be mapped with
    // map_page.

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
    memset(page_dir, 0, PAGE_TABLE_SIZE);

    kernel_pages = GETADDRESS(page_dir[PAGE_TABLE_SIZE-2]);

    // Fractal page mapping
    page_dir[PAGE_TABLE_SIZE-1] |= GETADDRESS(page_dir) | PAGE_PRESENT;

    map_kernel_pages(page_dir);

    enable_paging(page_dir);
    return page_dir;
}

uint32_t create_new_page_table(page_frame_t kernel_page) {
    page_frame_t *page_dir = alloc_frame();

    // Clear page directory
    memset(page_dir, 0, PAGE_TABLE_SIZE);

    // Map it into the kernel's master page table
    map_page(page_dir, page_dir, page_dir, 0);

    // Fractal page mapping
    page_dir[PAGE_TABLE_SIZE-1] |= GETADDRESS(page_dir) | PAGE_PRESENT;
    // Map kernel pages
    page_dir[PAGE_TABLE_SIZE-2] |= GETADDRESS(kernel_page) | PAGE_PRESENT;

    map_kernel_pages(page_dir);
    return page_dir;
}

void map_kernel_pages(uint32_t *page_dir) {
    // Put the kernel pages in the second to last slot of the page directory
    void *kernel_page_virtual_address = (PAGE_TABLE_SIZE - 22) << 22;

    // Map all those important bits
    for (page_frame_t i = KERNEL_START; i < KERNEL_END; i += PAGE_SIZE) {
        //map_page(page_dir, i, kernel_page_virtual_address, 0);
        map_page(page_dir, i, i, 0);
        kernel_page_virtual_address += PAGE_SIZE;
    }
    //map_page(page_dir, KHEAP_PHYS_ROOT, kernel_page_virtual_address, 0);
    map_page(page_dir, KHEAP_PHYS_ROOT, KHEAP_PHYS_ROOT, 0);
    kernel_page_virtual_address += PAGE_SIZE;

    //map_page(page_dir, VIDEO_MEMORY_BEGIN, kernel_page_virtual_address, 0);
    map_page(page_dir, VIDEO_MEMORY_BEGIN, (void*)VIDEO_MEMORY_BEGIN, 0);
    kernel_page_virtual_address += PAGE_SIZE;

    //map_page(page_dir, page_dir, kernel_page_virtual_address, 0);
    map_page(page_dir, page_dir, page_dir, 0);
    kernel_page_virtual_address += PAGE_SIZE;

}

int map_page(uint32_t *page_dir, page_frame_t physical_page,
        void *virtual_address, uint16_t permissions) {

    uint32_t *page_entry;
    uint32_t dir_index = HIGHINDEX(virtual_address);
    uint32_t entry_index = LOWINDEX(virtual_address);

    // Be sure.
    kassert(((uintptr_t)physical_page % PAGE_SIZE) == 0);
    kassert(((uintptr_t)virtual_address % PAGE_SIZE) == 0);


    // If the relevant page directory entry is absent, allocate it.
    if ((page_dir[dir_index] & 1) == 0) {
        page_entry = (uint32_t*)alloc_frame();

        page_dir[HIGHINDEX(page_entry)] =
            GETADDRESS(page_entry) | permissions | PAGE_PRESENT;

        page_dir[dir_index] = GETADDRESS(page_entry) | permissions |
            PAGE_PRESENT;
    } else {
        page_entry = (uint32_t*)GETADDRESS(page_dir[dir_index]);
    }
    page_entry[entry_index] = GETADDRESS(virtual_address) | permissions |
        PAGE_PRESENT;
    return 0;
}


int unmap_page(page_frame_t *page_dir, void *virtual_address) {

    uint32_t *page_entry;
    uint32_t dir_index = HIGHINDEX(virtual_address);
    uint32_t entry_index = LOWINDEX(virtual_address);

    if ((page_dir[dir_index] & 1) == 0) {
        return -1;
    }

    page_entry = (uint32_t*)GETADDRESS(page_dir[dir_index]);
    page_entry[entry_index] &= ~0x1;
    page_frame_t physical_page = GETADDRESS(page_entry[entry_index]);
    free_frame(physical_page);
    return 0;
}

void free_table(uint32_t *page_dir) {
    // The last two entries of the page directory are reserved. The last one
    // points to the directory itself, and the second to last one points to
    // kernel space
    for (size_t i = 0; i < PAGE_TABLE_SIZE-2; ++i) {
        uint32_t *page_entry = GETADDRESS(page_dir[i]);
        for (size_t j = 0; j < PAGE_TABLE_SIZE; ++j) {
            if ((page_entry[j] & PAGE_PRESENT) == 1) {
                free_frame(page_entry[j]);
            }
        }
    }
    free_frame(page_dir);
}
