#include <arch/x86/paging.h>

static struct page_entry *walk_page_directory(struct page_dir_entry *dir, const
        void *virtual_address, bool allocate);

// physical_address and virtual_address must be page aligned.
bool map_pages(struct page_dir_entry *dir, void *physical_address,
        void *virtual_address, uint32_t size, bool rw, bool super_user,
        bool disable_cache) {
    struct page_entry *page;
    kassert((uint32_t)physical_address % 4096 == 0);
    kassert((uint32_t)virtual_address % 4096 == 0);
    while (true) {
        page = walk_page_directory(dir, virtual_address, 1);
        if (page == 0) {
            return false;
        }
        page->address = (uint32_t)physical_address;
        page->present = 1;
        page->rw = rw;
        page->super_user = super_user;
        page->disable_cache = disable_cache;
        kassert(page->present);

        if (physical_address == virtual_address + size -1) {
            break;
        }
        virtual_address += PAGE_SIZE;
        physical_address += PAGE_SIZE;
    }
    return true;
}

struct page_dir_entry *kernel_page_table_install() {
    struct page_dir_entry *page_dir = kmalloc(PAGE_SIZE);
    if (page_dir == NULL) {
        return NULL;
    }
    // Zero page
    memset(page_dir, 0, PAGE_SIZE);
    bool err = map_pages(page_dir, (void *)KERNEL_START, (void *)KERNEL_START, KERNEL_SIZE,
            true, true, false);

    
    if (err == true) {
        return NULL;
    }
    enable_paging(page_dir);
    flush_tlb();
    return page_dir;
}

static struct page_entry *walk_page_directory(struct page_dir_entry *dir, const
        void *virtual_address, bool allocate) {
    struct page_entry *table;
    struct page_dir_entry *dir_entry = &dir[PAGE_TABLE_INDEX(virtual_address)];
    if (dir_entry->present) {
        table = (struct page_entry*)PHYSICAL_TO_VIRTUAL((uintptr_t)dir_entry->address);
    } else {
        if (!allocate) {
            return NULL;
        }
        table = kmalloc(PAGE_SIZE);
        if (table == NULL) {
            return NULL;
        }
        memset(table, 0, PAGE_SIZE);
        dir_entry->address = (uint32_t)virtual_address;
        dir_entry->present = 1;
        dir_entry->rw = 1;
        dir_entry->super_user = 0;
    }
    return &table[PAGE_TABLE_INDEX(virtual_address)];
}
