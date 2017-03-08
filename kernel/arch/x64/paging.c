#include <arch/x64/control_registers.h>
#include <arch/x64/paging.h>
#include <truth/log.h>
#include <truth/physical_allocator.h>
#include <truth/panic.h>
#include <truth/slab.h>
#include <truth/string.h>
#include <truth/types.h>

extern void invalidate_tlb(void);
extern uint64_t invalidate_page(void *);

#define pl1_Count 512
#define pl2_Count 512
#define pl3_Count 512
#define pl4_Count 512

#define pl1_offset 12
#define pl1_mask   0777
#define pl2_offset 21
#define pl2_mask   0777
#define pl3_offset 30
#define pl3_mask   0777
#define pl4_offset 39
#define pl4_mask   0777

#define align_page(x) (x & ~0xfff)

// Any even number will do
#define free_page_entry 0xccccccccccccccul

#define Phys_Addr_Mask 0x000ffffffffff000

typedef uint64_t pl4_entry;
typedef uint64_t pl3_entry;
typedef uint64_t pl2_entry;
typedef uint64_t pl1_entry;

struct page_table {
    pl4_entry entries[pl4_Count];
};

phys_addr page_entry_to_phys(uint64_t entry) {
    return entry & Phys_Addr_Mask;
}

static inline size_t pl4_index(void *address) {
    return (uintptr_t)address >> pl4_offset & pl4_mask;
}

static inline size_t pl3_index(void *address) {
    return ((uintptr_t)address >> pl3_offset) & pl3_mask;
}

static inline size_t pl2_index(void *address) {
    return ((uintptr_t)address >> pl2_offset) & pl2_mask;
}

static inline size_t pl1_index(void *address) {
    return ((uintptr_t)address >> pl1_offset) & pl1_mask;
}

static inline phys_addr table_phys_address(struct page_table *page_table) {
    return (phys_addr)(uintptr_t)page_table;
}

static struct page_table *current_page_table(void) {
    return (struct page_table *)01777777777777777770000;
}

#define page_size (KB * 4)

static pl3_entry *get_pl3_index(size_t pl4_index) {
    return (pl3_entry *)(01777777777777770000000 | (pl4_index << 12));
}

static pl2_entry *get_pl2_index(size_t pl4_index, size_t pl3_index) {
    return (pl2_entry *)(01777777777770000000000 |
                   (pl4_index << 21) |
                   (pl3_index << 12));
}

static pl1_entry *get_pl1_index(size_t pl4_index, size_t pl3_index,
                          size_t pl2_index) {
    return (pl1_entry *)(01777777770000000000000 |
                   (pl4_index << 30) |
                   (pl3_index << 21) |
                   (pl2_index << 12));
}

static pl3_entry *get_pl3(void *address) {
    return get_pl3_index(pl4_index(address));
}

static pl2_entry *get_pl2(void *address) {
    return get_pl2_index(pl4_index(address), pl3_index(address));
}

static pl1_entry *get_pl1(void *address) {
    return get_pl1_index(pl4_index(address), pl3_index(address), pl2_index(address));
}


static inline bool is_pl3_present(struct page_table *page_table,
                                  void *address) {
    return (page_table->entries[pl4_index(address)] & Memory_Present) == 1;
}

static inline bool is_pl2_present(pl3_entry *level_three, void *address) {
    return level_three[pl3_index(address)] & Memory_Present;
}

static inline bool is_pl1_present(pl2_entry *level_two, void *address) {
    return level_two[pl2_index(address)] & Memory_Present;
}

static inline bool is_Memory_Present(pl1_entry *level_one, void *address) {
    return level_one[pl1_index(address)] & Memory_Present;
}

void debug_paging(void) {
    struct page_table *page_table = current_page_table();
    for (size_t i = 0; i < pl4_Count; ++i) {
        if (page_table->entries[i] * Memory_Present) {
            logf(Log_Debug, "%zu has %lx\n", i, page_table->entries[i]);
        }
    }
    logf(Log_Debug, "kernel start: %p\n", &__kernel_start);
    logf(Log_Debug, "kernel end: %p\n", &__kernel_end);
}

enum status checked map_page(void *virtual_address, phys_addr phys_address,
                             enum memory_attributes permissions) {

    struct page_table *page_table = current_page_table();
    logf(Log_Debug, "Mapping: %p -> %lx\n", virtual_address, phys_address);
    if (!is_pl3_present(page_table, virtual_address)) {
        phys_addr phys_address = physical_alloc(1);
        page_table->entries[pl4_index(virtual_address)] =
            (phys_address | permissions | Memory_User_Access |
             Memory_Present);
        invalidate_tlb();
    }

    pl3_entry *level_three = get_pl3(virtual_address);
    if (!is_pl2_present(level_three, virtual_address)) {
        phys_addr phys_address = physical_alloc(1);
        level_three[pl3_index(virtual_address)] =
            (phys_address | permissions | Memory_User_Access |
             Memory_Present);
        invalidate_tlb();
    }

    pl2_entry *level_two = get_pl2(virtual_address);
    if (!is_pl1_present(level_two, virtual_address)) {
        phys_addr phys_address = physical_alloc(1);
        level_two[pl2_index(virtual_address)] =
            (phys_address | permissions | Memory_User_Access |
             Memory_Present);
        invalidate_tlb();
    }

    pl1_entry *level_one = get_pl1(virtual_address);
    if (is_Memory_Present(level_one, virtual_address)) {
        logf(Log_Debug, "The virtual address %p is already present\n",
             virtual_address);
        return Error_Present;
    }

    level_one[pl1_index(virtual_address)] =
        (phys_address | permissions | Memory_Present);
    invalidate_tlb();

    int *test = virtual_address;
    logf(Log_Debug, "Testing: %x\n", test[0]);

    return Ok;
}

void unmap_page(void *address, bool free_physical_memory) {
    struct page_table *page_table = current_page_table();
    if (is_pl3_present(page_table, address) &&
        is_pl2_present(get_pl3(address), address) &&
        is_pl1_present(get_pl2(address), address)) {
        pl1_entry *level_one = get_pl1(address);


        if (free_physical_memory) {
            physical_free(align_page(level_one[pl1_index(address)]), 1);
        }
        level_one[pl1_index(address)] = free_page_entry;
        invalidate_page(address);
    }
}

enum status map_external_page(struct page_table *page_table,
                              void *virtual_address, phys_addr phys_address,
                              enum memory_attributes permissions) {
    phys_addr original_paging = read_cr3();
    write_cr3(table_phys_address(page_table));
    enum status status = map_page(virtual_address, phys_address, permissions);
    write_cr3(original_paging);
    return status;
}

void unmap_external_page(struct page_table *page_table, void *virtual_address,
                         bool free_physical_memory) {
    phys_addr original_paging = read_cr3();
    write_cr3(table_phys_address(page_table));
    unmap_page(virtual_address, free_physical_memory);
    write_cr3(original_paging);
}

void switch_page_table(struct page_table *page_table) {
    write_cr3(table_phys_address(page_table));
}

struct page_table *page_table_init(void) {
    phys_addr pt_phys;
    struct page_table *pt = slab_alloc_phys(&pt_phys, Memory_Writable);
    if (pt == NULL) {
        return NULL;
    }
    memset(pt->entries, 0, pl4_Count / 2);
    memcpy(&pt->entries[Page_Small / 2], current_page_table(), pl4_Count / 2);
    return pt;
}
