#include <arch/x86/paging.h>

void kernel_page_table_install() {
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

    uint32_t *page_dir = (uint32_t*)PAGE_DIRECTORY;
    memset(page_dir, 0, PAGE_TABLE_SIZE);

    // Fractal page mapping
    page_dir[PAGE_TABLE_SIZE-1] |= GETADDRESS(page_dir) | PAGE_PRESENT;

    // Map all those important bits
    for (page_frame_t i = KERNEL_START; i < KERNEL_END; i += PAGE_SIZE) {
        map_page(i, (void*)i, 0);
    }
    map_page(KHEAP_PHYS_ROOT, (void*)KHEAP_PHYS_ROOT, 0);
    map_page(VIDEO_MEMORY_BEGIN, (void*)VIDEO_MEMORY_BEGIN, 0);
    map_page(PAGE_DIRECTORY, (void*)PAGE_DIRECTORY, 0);

    enable_paging(page_dir);
}

int map_page(page_frame_t physical_page, void *virtual_address,
        uint16_t permissions) {

    uint32_t *page_entry;
    uint32_t *page_dir = (uint32_t*)PAGE_DIRECTORY;
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
    page_entry[entry_index] |= GETADDRESS(virtual_address) | permissions |
        PAGE_PRESENT;
    return 0;
}
