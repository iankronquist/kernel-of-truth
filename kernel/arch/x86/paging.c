#include <arch/x86/paging.h>

#include <truth/kassert.h>
#include <truth/kabort.h>
#include <truth/klog.h>
#include <truth/kmem.h>
#include <truth/pmem.h>
#include <truth/string.h>
#include <truth/types.h>

#include <truth/private/memlayout.h>

#define USED_PDE 0xcccccccc

struct page_table {
    uint64_t page_directory[PDPT_COUNT];
};

static uint64_t *get_page_dir_address(void *address) {
    return (uint64_t*)(((uintptr_t)address & PDPT_MASK) |
        ((PAGE_DIR_ENTRY_COUNT-1) << PD_SHIFT) |
        ((PAGE_TABLE_ENTRY_COUNT-1) << PT_SHIFT));
}

static uint64_t *get_page_table_address(void *address) {
    return (uint64_t*)(((uintptr_t)address & PDPT_MASK) |
        ((PAGE_DIR_ENTRY_COUNT-1) << PD_SHIFT));
}

static inline phys_addr_t entry_to_phys_addr(uint64_t p) {
    return p & ~0xfff;
}

static inline bool is_present(uint64_t entry) {
    return entry & perm_present;
}


status_t checked map_page(struct page_table *cur_table, void *va,
        phys_addr_t pa, enum memory_permissions perms) {
    uint64_t *pd, *pt, page_dir_entry, page_table_entry;

    kassert(PAGE_ALIGN(va) == (uintptr_t)va);
    kassert(PAGE_ALIGN(pa) == pa);

    size_t pdpt_index = page_dir_index(va);
    if (!is_present(cur_table->page_directory[pdpt_index])) {
        page_dir_entry = alloc_frame();
        cur_table->page_directory[pdpt_index] = page_dir_entry | perm_write |
            perm_present;
        memset(get_page_dir_address(va), 0, PAGE_SIZE);
    }

    pd = get_page_dir_address(va);
    size_t pd_index = page_dir_index(va);
    if (!is_present(pd[pd_index])) {
        page_table_entry = alloc_frame();
        pd[pd_index] = page_table_entry | perm_write | perm_present;
        memset(get_page_table_address(va), 0, PAGE_SIZE);
    }

    pt = get_page_table_address(va);
    size_t pt_index = page_table_index(va);
    if (is_present(pt[pt_index])) {
        return Err;
    }

    pt[pt_index] = pa | perms | perm_present;

    return Ok;
}

status_t checked unmap_page(struct page_table *cur_table, void *va,
        bool free_phys_addr) {
    uint64_t *pd, *pt;

    kassert(PAGE_ALIGN(va) == (uintptr_t)va);

    size_t pdpt_index = page_dir_index(va);
    if (!is_present(cur_table->page_directory[pdpt_index])) {
        return Err;
    }

    pd = get_page_dir_address(va);
    size_t pd_index = page_dir_index(va);
    if (!is_present(pd[pd_index])) {
        return Err;
    }

    pt = get_page_table_address(va);
    size_t pt_index = page_table_index(va);
    if (!is_present(pt[pt_index])) {
        return Err;
    } else {
        if (free_phys_addr) {
            free_frame(entry_to_phys_addr(pt[pt_index]));
        }
        pt[pt_index] = USED_PDE;
    }

    return Ok;
}

status_t checked create_page_table(struct page_table **pt) {
    uint64_t page_dir;
    struct page_table *new_pt;

    new_pt = kmalloc_aligned(sizeof(struct page_table), 4);
    if (new_pt == NULL) {
        return Err;
    }
    // Create top level PDPT structure.
    for (size_t i = 0; i < PDPT_COUNT; ++i) {
        page_dir = alloc_frame();
        if (page_dir == 0) {
            kfree(new_pt);
            return Err;
        }
        new_pt->page_directory[i] = page_dir | perm_present | perm_write;
    }
    *pt = new_pt;
    return Ok;
}


extern struct page_table bootstrap_pdpt;
// FIXME: When I turn multiprocess back on, remember to fetch this from the
// current process.
struct page_table *get_cur_page_table(void) {
    struct page_table *table = &bootstrap_pdpt;
    return table;
}
