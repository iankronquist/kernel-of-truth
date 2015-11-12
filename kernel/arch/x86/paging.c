#include <arch/x86/paging.h>

void map_page(void *physical_address, void *virtual_address, bool present, bool rw, bool write_cache, bool accessed, bool size) {
    kassert(physical_address % PAGE_SIZE == 0);
    kassert(virtual_address % PAGE_SIZE == 0);

struct page_dir_entry {
    uint32_t present:1;
    uint32_t rw:1;
    uint32_t super_user:1;
    uint32_t write_cache:1;
    uint32_t disable_cache:1;
    uint32_t accessed:1;
    uint32_t zero:1;
    uint32_t size:1; // Set to 0 for 4kb
    uint32_t unused:4;
    uint32_t address:20;
}__attribute__((packed));

struct page_entry {
    uint32_t present:1;
    uint32_t rw:1;
    uint32_t super_user:1;
    uint32_t write_cache:1;
    uint32_t disable_cache:1;
    uint32_t accessed:1;
    uint32_t dirty:1;
    uint32_t unused:5;
    uint32_t address:20;
}__attribute__((packed));


//    uint64_t page_dir_index = (uint64_t)virtual_address >> 22;
//    uint64_t page_index = (uint64_t)virtual_address >> 12 & MAGIC_NUMBER;
}
