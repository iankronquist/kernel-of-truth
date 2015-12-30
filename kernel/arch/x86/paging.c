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
    kprintf("page dir %p\n", page_dir);
    return page_dir;
}

uint32_t create_new_page_dir(page_frame_t *cur_page_dir,
        page_frame_t kernel_page, void *initial_location) {

    kputs("create new pd");
    __asm__ volatile ("cli");
    page_frame_t *page_dir = alloc_frame();
    //uint32_t *page_dir_addr = just_give_me_a_page(cur_page_dir, page_dir, 0);
    /*
    kprintf("%p\n", page_dir_addr);
    kprintf("%p\n", page_dir);
    kprintf("%p\n", cur_page_dir);
    kassert(page_dir_addr != ~0);
    kputs("here");

    kassert((cur_page_dir[TOP10(page_dir_addr)] & PAGE_PRESENT) == 1);
    page_frame_t *pe = cur_page_dir[TOP10(page_dir_addr)] ^ PAGE_PRESENT;
    kassert((pe[((uint32_t)page_dir_addr >> 12) & 0x3ff] & PAGE_PRESENT) == 1);
    */

    // Clear page directory
    //memset(page_dir_addr, 0, PAGE_SIZE);
    memset(page_dir, 0, PAGE_SIZE);
    kputs("there");

    // Fractal page mapping
    //page_dir_addr[PAGE_TABLE_SIZE-1] = GETADDRESS(page_dir) | PAGE_PRESENT;
    // Map kernel pages
    //page_dir_addr[PAGE_TABLE_SIZE-2] = GETADDRESS(kernel_page) | PAGE_PRESENT;

    // Fractal page mapping
    page_dir[PAGE_TABLE_SIZE-1] = GETADDRESS(page_dir) | PAGE_PRESENT;
    // Map kernel pages
    //page_dir[PAGE_TABLE_SIZE-2] = GETADDRESS(kernel_page) | PAGE_PRESENT;

    kputs("map k p");
    map_kernel_pages(page_dir);

    //unmap_page(page_dir_addr);
    kputs("tell");
    tell_me_a_bit(page_dir);

    __asm__ volatile ("sti");

    return page_dir;
}

void tell_me_a_bit(uint32_t *page_dir) {
    kprintf("{");
    for (size_t i = 0; i < PAGE_TABLE_SIZE; ++i) {
        if (page_dir[i] & PAGE_PRESENT) {
            kprintf("%p -> ", i);
            kprintf("%p, ", page_dir[i]);
        }
    }
    kputs("}");
}


void *just_give_me_a_page(uint32_t *page_dir, page_frame_t phys_addr,
        uint16_t permissions) {
    kassert(is_free_frame(phys_addr));
    kassert(phys_addr % PAGE_SIZE == 0);
    // Walk the page directory in search of an available address
    for (size_t i = 0; i < PAGE_TABLE_SIZE-2; ++i) {
        if (page_dir[i] & PAGE_PRESENT) {
        kprintf("%i\n", i);
            uint32_t *page_entry = GETADDRESS(page_dir[i]);
            // FIXME problems here
            page_dir[HIGHINDEX(page_entry)] = GETADDRESS(page_entry) |
                PAGE_PRESENT;
        kprintf("%p\n", page_dir[i]);
        kprintf("%p\n", page_entry);
            for (size_t j = 0; j < PAGE_TABLE_SIZE-1; ++j) {
                if (!(page_entry[j] & PAGE_PRESENT)) {
                    //kputs("found");
                    kprintf("addr %p, %p, %p, %p\n", i, j, (i << 22) | (j << 12), phys_addr);
                    //page_entry[i] = phys_addr | permissions | PAGE_PRESENT;
                    page_entry[j] = phys_addr | PAGE_PRESENT;
                    //kprintf("%p\n", page_entry[j]);
                    //kprintf("%p\n", page_dir[i]);
                    use_frame(phys_addr);
                    flush_tlb();
                    return (i << 22) | (j << 12);
                }
            }
        }

    }

    // Allocate a new page table and return its first address
    for (size_t i = 0; i < PAGE_TABLE_SIZE-2; ++i) {
        if (!(page_dir[i] & PAGE_PRESENT)) {
            uint32_t *page_table = alloc_frame();
            page_dir[i] = (uint32_t)page_table | PAGE_PRESENT;
            page_table[0] = phys_addr | permissions | PAGE_PRESENT;
            kputs("allocating");
            kprintf("addr %p\n", (i << 22) | 0);
            use_frame(phys_addr);
            flush_tlb();
            return (1 << 22) | (0);
        }
    }

    // The page table is full. Fail by returning a special unaligned address
    // (all 1s)
    return ~0;
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

    // See fractal paging
    //map_page(page_dir, page_dir, kernel_page_virtual_address, 0);
    //map_page(page_dir, page_dir, page_dir, 0);
    //kernel_page_virtual_address += PAGE_SIZE;
    //tell_me_a_bit(page_dir);

}

uint32_t *clone_directory(uint32_t *page_dir) {
    uint32_t *new_dir_phys = alloc_frame();
    // FIXME: Assume the physical address of new_dir_phys is not already taken!
    //map_page(page_dir, new_dir_phys, new_dir_phys, 0);
    uint32_t *new_dir = just_give_me_a_page(page_dir, new_dir_phys, 0);
    memset(new_dir, 0, PAGE_SIZE);
    for (size_t i = 0; i < PAGE_TABLE_SIZE; ++i) {
        if (page_dir[i] & PAGE_PRESENT) {
            uint32_t *old_entry = GETADDRESS(page_dir[i]);
            uint32_t *new_entry = alloc_frame();
            uint32_t *entry = just_give_me_a_page(page_dir, new_entry, 0);
            new_dir[i] = (uint32_t)new_entry | (page_dir[i] & 0xfff);
            for (size_t j = 0; j < PAGE_TABLE_SIZE; ++j) {
                entry[j] = old_entry[i];
            }
        }
    }
    return new_dir_phys;
}

// TODO: refactor to use get_page
int map_page(uint32_t *page_dir, page_frame_t physical_page,
        void *virtual_address, uint16_t permissions) {

    //kprintf("%p -> %p ::: ", virtual_address, physical_page);
    uint32_t *page_entry;
    uint32_t dir_index = HIGHINDEX(virtual_address);
    uint32_t entry_index = LOWINDEX(virtual_address);

    // Be sure.
    kassert(((uintptr_t)physical_page % PAGE_SIZE) == 0);
    kassert(((uintptr_t)virtual_address % PAGE_SIZE) == 0);

    // If the relevant page directory entry is absent, allocate it.
    if ((page_dir[dir_index] & 1) == 0) {
        page_entry = (uint32_t*)alloc_frame();
        //kprintf("\nmp alloced %p -> %p\n", (dir_index << 22)|(entry_index <<12), page_entry);

        // FIXME: this can cause problems if something is already mapped at the
        // address of the page entry.
        kassert((page_dir[HIGHINDEX(page_entry)] & PAGE_PRESENT) == 0);
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


// TODO: refactor to use get_page
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

uint32_t *get_page_entry(page_frame_t *page_dir, void *virtual_address) {
    uint32_t *page_entry;
    uint32_t dir_index = HIGHINDEX(virtual_address);
    uint32_t entry_index = LOWINDEX(virtual_address);

    if ((page_dir[dir_index] & 1) == 0) {
        return ~0; // return all fs, an invalid value
    }

    page_entry = (uint32_t*)GETADDRESS(page_dir[dir_index]);
    return page_entry;
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

uint32_t map_temporary(uint32_t *page_dir, uint32_t **page_dir_addr) {
    *page_dir_addr = ((PAGE_TABLE_SIZE-1) << 22) | 0;
    uint32_t old_value = kernel_pages[0];
    kernel_pages[0] = GETADDRESS(page_dir) | PAGE_PRESENT;
    return old_value;
}

void unmap_temporary(uint32_t *old_value) {
    kernel_pages[0] = old_value;
}
