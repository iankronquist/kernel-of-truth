#include <arch/x86/paging.h>
#include <test.h>

static void loader_stub(void *dest, void *src) {
    // Ignore src
    src++;
    memcpy(dest, a, sizeof(a));
    //memcpy(dest, src, 100);
}

page_frame_t kernel_page_table_install() {
    // Certain very important things already exist in physical memory. They
    // need to be marked as present so that the allocator doesn't grab them by
    // accident.
    // After they are marked as present they can safely be mapped with
    // map_page.
    use_frame(0);

    // Mark all the pages the kernel sits on as used
    use_range(KERNEL_START, KERNEL_END);
    // Mark the pages which ACPI sits on as used
    use_range(ACPI_BEGIN, ACPI_END);

    // Mark the kernel heap as in use
    use_frame(KHEAP_PHYS_ROOT);
    // Mark video memory as in use
    use_frame(VIDEO_MEMORY_BEGIN);
    // Mark the paging directory as in use
    use_frame(PAGE_DIRECTORY);

    page_frame_t page_dir = alloc_frame();
    uint32_t *page_entries = (uint32_t *)page_dir;
    memset(page_entries, 0, PAGE_SIZE);

    // Fractal page mapping
    page_entries[PAGE_TABLE_SIZE-1] = GETADDRESS(page_dir) | PAGE_PRESENT;

    map_kernel_pages(page_entries);

    enable_paging(page_dir);
    return page_dir;
}

page_frame_t create_page_dir(void *link_loc, void *stack_loc,
        void *user_stack_loc, void(*entrypoint)(), uint16_t permissions) {

    // Allocate a physical address for the new page table
    page_frame_t page_table = alloc_frame();

    // Map the new page table into the current page table.
    uint32_t *page_entries = find_free_addr(CUR_PAGE_DIRECTORY_ADDR,
            page_table, 0);

    // Zero page table to make sure present bits are clear
    memset(page_entries, 0, PAGE_SIZE);

    // Set up fractal mapping
    page_entries[PAGE_TABLE_SIZE-1] = GETADDRESS(page_table) | PAGE_PRESENT;

    // Install kernel pages into the table
    inner_map_kernel_pages(page_entries);

    // Allocate a kernel stack and the location where the code will live
    page_frame_t link_page = alloc_frame();
    page_frame_t stack_page = alloc_frame();
    page_frame_t user_stack_page = alloc_frame();
    inner_map_page(page_entries, link_page, link_loc, permissions);
    inner_map_page(page_entries, stack_page, stack_loc, permissions);
    inner_map_page(page_entries, user_stack_page, user_stack_loc, permissions);

    // Runtime test
    page_frame_t orig_page_table = get_page_dir();
    klog("Loading new page table to make sure it is well formed\n");
    enable_paging(page_table);
    // Set up the stack for the initial restore
    // Zero the registers
    memset(&((uint32_t*)stack_loc)[PAGE_TABLE_SIZE-44], 0, 44);
    // Set the eip to the link location
    ((uint32_t*)stack_loc)[PAGE_TABLE_SIZE-1] = (uint32_t)link_loc;
    // Set the esp to the top of the stack
    ((uint32_t*)stack_loc)[PAGE_TABLE_SIZE-5] = (uint32_t)(stack_loc+PAGE_TABLE_SIZE-1);
    // Set the data segment selector to ring 0.
    // FIXME: I don't use this right when I call switch_users_mode_task.
    ((uint32_t*)stack_loc)[PAGE_TABLE_SIZE-44] = RING_0_DATA_SELECTOR;

    // This is where I would call the loader, but I don't have one yet.
    loader_stub(link_loc, entrypoint);
    klog("Yep, it's valid. Reverting to old page table.\n");
    enable_paging(orig_page_table);

    // Unmap the new page table from the current page table
    inner_unmap_page(CUR_PAGE_DIRECTORY_ADDR, page_entries, false);

    return page_table;
}

void *find_free_addr(uint32_t *page_entries, page_frame_t phys_addr,
        uint16_t permissions) {
    // Walk the page directory in search of an available address
    for (size_t i = 0; i < PAGE_TABLE_SIZE-1; ++i) {
        if (page_entries[i] & PAGE_PRESENT) {
            uint32_t *page_entry = (uint32_t*)(PAGING_DIR_PHYS_ADDR+i);
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
    for (size_t i = 0; i < PAGE_TABLE_SIZE-1; ++i) {
        if (!(page_entries[i] & PAGE_PRESENT)) {
            page_frame_t page_table_frame = alloc_frame();
            uint32_t *page_table = (uint32_t*)(PAGING_DIR_PHYS_ADDR+i);
            page_entries[i] = page_table_frame | PAGE_PRESENT;
            memset(page_table, 0, PAGE_SIZE);
            page_table[0] = phys_addr | permissions | PAGE_PRESENT;
            flush_tlb();
            return (void*)((1 << 22) | (0));
        }
    }

    // The page table is full. Fail by returning a special unaligned address
    return (void*)EPHYSMEMFULL;
}


void map_kernel_pages(uint32_t *page_dir) {
    map_page(page_dir, 0x100000, (void*)0x100000, 0);
    // Map all those important bits
    for (page_frame_t i = KERNEL_START; i < KERNEL_END; i += PAGE_SIZE) {
        map_page(page_dir, i, (void*)i, 0);
    }
    map_page(page_dir, KHEAP_PHYS_ROOT, (void*)KHEAP_PHYS_ROOT, 0);

    // FIXME: Move to video memory driver
    map_page(page_dir, VIDEO_MEMORY_BEGIN, (void*)VIDEO_MEMORY_BEGIN, 0);
}

void inner_map_kernel_pages(uint32_t *page_dir) {
    inner_map_page(page_dir, 0x100000, (void*)0x100000, 0);
    // Map all those important bits
    for (page_frame_t i = KERNEL_START; i < KERNEL_END; i += PAGE_SIZE) {
        inner_map_page(page_dir, i, (void*)i, 0);
    }
    inner_map_page(page_dir, KHEAP_PHYS_ROOT, (void*)KHEAP_PHYS_ROOT, 0);

    // FIXME: Move to video memory driver
    inner_map_page(page_dir, VIDEO_MEMORY_BEGIN, (void*)VIDEO_MEMORY_BEGIN, 0);
}

int map_page(uint32_t *page_dir, page_frame_t physical_page,
        void *virtual_address, uint16_t permissions) {

    kassert((physical_page % PAGE_SIZE) == 0);
    kassert(((uintptr_t)virtual_address % PAGE_SIZE) == 0);

    uint32_t *page_entry;
    uint32_t dir_index = HIGHINDEX(virtual_address);
    uint32_t entry_index = LOWINDEX(virtual_address);

    // If the relevant page directory entry is absent, allocate it.
    if ((page_dir[dir_index] & PAGE_PRESENT) == 0) {
        page_frame_t page_frame = alloc_frame();
        page_entry = (uint32_t*)(PAGING_DIR_PHYS_ADDR+dir_index);
        memset(page_entry, 0, PAGE_SIZE);

        page_dir[dir_index] = GETADDRESS(page_frame) | permissions |
            PAGE_PRESENT;
    } else {
        page_dir[dir_index] |= permissions;
        page_entry = (uint32_t*)GETADDRESS(page_dir[dir_index]);
    }
    page_entry[entry_index] = GETADDRESS(virtual_address) | permissions |
        PAGE_PRESENT;
    return 0;
}

int inner_map_page(uint32_t *page_dir, page_frame_t physical_page,
        void *virtual_address, uint16_t permissions) {
    uint32_t *cur_page_dir = CUR_PAGE_DIRECTORY_ADDR;
    kassert((physical_page % PAGE_SIZE) == 0);
    kassert(((uintptr_t)virtual_address % PAGE_SIZE) == 0);

    uint32_t *page_entry;
    uint32_t dir_index = HIGHINDEX(virtual_address);
    uint32_t entry_index = LOWINDEX(virtual_address);

    // If the relevant page directory entry is absent, allocate it.
    if ((page_dir[dir_index] & PAGE_PRESENT) == 0) {
        page_frame_t page_frame = alloc_frame();
        page_entry = find_free_addr(cur_page_dir, page_frame, 0);
        memset(page_entry, 0, PAGE_SIZE);

        page_dir[dir_index] = GETADDRESS(page_frame) | permissions |
            PAGE_PRESENT;
    } else {
        page_dir[dir_index] |= permissions;
        page_entry = (uint32_t*)GETADDRESS(page_dir[dir_index]);
        page_entry = find_free_addr(cur_page_dir, GETADDRESS(page_dir[dir_index]), 0);
    }
    page_entry[entry_index] = GETADDRESS(virtual_address) | permissions |
        PAGE_PRESENT;
    inner_unmap_page(cur_page_dir, page_entry, false);
    return 0;
}

int inner_unmap_page(uint32_t *page_entries, void *virtual_address,
        bool should_free_frame) {

    uint32_t *page_entry;
    uint32_t dir_index = HIGHINDEX(virtual_address);
    uint32_t entry_index = LOWINDEX(virtual_address);

    // Original top level page table was not mapped. Return error.
    if ((page_entries[dir_index] & 1) == 0) {
        return -1;
    }
    page_entry = (uint32_t*)(FRACTAL_MAP + PAGE_SIZE * dir_index);
    // Clear present bit
    page_entry[entry_index] &= ~PAGE_PRESENT;

    if (should_free_frame) {
        page_frame_t physical_page = GETADDRESS(page_entry[entry_index]);
        free_frame(physical_page);
    }

    return 0;
}

void free_table(uint32_t *page_dir) {
    // The last entry of the page directory is reserved. It points to the page
    // table itself.
    for (size_t i = 0; i < PAGE_TABLE_SIZE-2; ++i) {
        uint32_t *page_entry = (uint32_t*)GETADDRESS(page_dir[i]);
        for (size_t j = 0; j < PAGE_TABLE_SIZE; ++j) {
            uintptr_t addr = (i<<20|j<<12);
            if (addr == VIDEO_MEMORY_BEGIN ||
                    (addr >= KERNEL_START && addr < KERNEL_END)) {
                continue;
            }
            if ((page_entry[j] & PAGE_PRESENT) == 1) {
                free_frame(page_entry[j]);
            }
        }
    }
    free_frame((page_frame_t)page_dir);
}
