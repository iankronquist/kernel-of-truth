#pragma once

#include <truth/types.h>
#include <truth/memtypes.h>

#include <contrib/multiboot.h>

struct page_table;

#define PDPT_COUNT             4
#define PAGE_DIR_ENTRY_COUNT   512
#define PAGE_TABLE_ENTRY_COUNT 512

#define PT_SHIFT   12
#define PD_SHIFT   21
#define PDPT_SHIFT 30

#define PT_MASK    0x001ff000
#define PD_MASK    0x3fe00000
#define PDPT_MASK  0xc0000000

enum memory_permissions {
    perm_none           = 0,
    perm_present        = (1ull << 0),
    perm_write          = (1ull << 1),
    perm_user_mode      = (1ull << 2),
    perm_write_through  = (1ull << 3),
    perm_uncached       = (1ull << 4),
    perm_not_executable = (1ull << 63),
};

static inline size_t pdpt_index(void *addr) {
    return (size_t)((uintptr_t)addr >> PDPT_SHIFT);
}

static inline size_t page_dir_index(void *addr) {
    return (size_t)(((uintptr_t)addr & PD_MASK) >> PD_SHIFT);
}

static inline size_t page_table_index(void *addr) {
    return (size_t)(((uintptr_t)addr & PT_MASK) >> PT_SHIFT);
}


status_t checked map_page(struct page_table*, void*, phys_addr_t,
        enum memory_permissions);

status_t checked unmap_page(struct page_table *pt, void *va,
        bool free_phys_addr);

status_t checked create_page_table(struct page_table**);

struct page_table *get_cur_page_table(void);
