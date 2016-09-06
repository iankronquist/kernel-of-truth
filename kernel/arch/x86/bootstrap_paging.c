#include <truth/klog.h>
#include <truth/types.h>
#include <truth/memtypes.h>
#include <truth/private/memlayout.h>

#define PDPT_COUNT             4
#define PAGE_DIR_ENTRY_COUNT   512
#define PAGE_TABLE_ENTRY_COUNT 512

#define PT_SHIFT   12
#define PD_SHIFT   21
#define PDPT_SHIFT 30

#define PT_MASK  0x001ff000
#define PD_MASK  0x3fe00000

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

extern uint64_t bootstrap_pdpt[PDPT_COUNT];
extern uint64_t bootstrap_page_dir[PAGE_DIR_ENTRY_COUNT];
extern uint64_t bootstrap_page_table[PAGE_DIR_ENTRY_COUNT];

void map_range(phys_addr_t start, phys_addr_t end,
        enum memory_permissions perms) {
    for (phys_addr_t page = start; page < end; page += PAGE_SIZE) {
        uintptr_t phys_addr = page;
        void *virt_addr = (void*)phys_addr;
        bootstrap_page_table[page_table_index(virt_addr)] = phys_addr | perms;
    }
}

void bootstrap_paging(void) {
    // We assume that the kernel will fit on one page directory entry.
    // Point PDPT to PD.
    bootstrap_pdpt[pdpt_index((void*)BOOTSTRAP_START)] =
        (uintptr_t)bootstrap_page_dir | perm_present | perm_write;
    // Point PD to PT.
    bootstrap_page_dir[page_dir_index((void*)BOOTSTRAP_START)] =
        (uintptr_t)bootstrap_page_table | perm_present | perm_write;
    map_range(BOOTSTRAP_START, BOOTSTRAP_END, perm_present);
    map_range(TEXT_START, TEXT_END, perm_present);
    map_range(DATA_START, DATA_END, perm_write | perm_present | perm_write);
    map_range(RODATA_START, RODATA_END, perm_present);
    map_range(BSS_START, BSS_END, perm_write | perm_present | perm_write);

    // Fractal paging.
    bootstrap_page_dir[PAGE_DIR_ENTRY_COUNT-1] =
        (uintptr_t)bootstrap_page_dir | perm_present;

    // Unmap stack canary.
    bootstrap_page_table[page_table_index((void*)KERNEL_START +
            BOOTSTRAP_STACK_CANARY_START)] = perm_none;
}
