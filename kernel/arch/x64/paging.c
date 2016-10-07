#include <arch/x64/paging.h>
#include <truth/log.h>
#include <truth/physical_allocator.h>
#include <truth/panic.h>
#include <truth/types.h>

extern void invalidate_tlb(void);
extern uint64_t invalidate_page(void *);

#define pl1_count 512
#define pl2_count 512
#define pl3_count 512
#define pl4_count 512

#define pl1_offset 12
#define pl1_mask   0x1ff
#define pl2_offset 21
#define pl2_mask   0x1ff
#define pl3_offset 30
#define pl3_mask   0x1ff
#define pl4_offset 39
#define pl4_mask   0x1ff

#define align_page(x) (x & ~0xfff)

// Any even number will do
#define free_page_entry 0xccccccccccccccul

typedef uint64_t pl4_entry;
typedef uint64_t pl3_entry;
typedef uint64_t pl2_entry;
typedef uint64_t pl1_entry;

typedef pl4_entry pl4[pl4_count];
typedef pl3_entry pl3[pl3_count];
typedef pl2_entry pl2[pl2_count];
typedef pl1_entry pl1[pl1_count];

struct page_table {
    pl4 entries;
};

extern void write_cr3(phys_addr value);
extern uint64_t read_cr3(void);

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
    return (struct page_table *)0xfffffffffffff000;
}

#define fractal_region 0xffffff8000000000

#define page_size 4 * KB

static pl3 *get_pl3(void *address) {
    return (pl2 *)(0xffffffffffe00000 | page_size * pl4_index(address));
}

static pl2 *get_pl2(void *address) {
    return (pl2 *)(0xffffffffc0000000 |
                   pl3_count * page_size * pl4_index(address) |
                   page_size * pl3_index(address));
}

static pl1 *get_pl1(void *address) {
    return (pl1 *)(0xffffff8000000000 |
                   pl3_count * pl2_count * page_size * pl4_index(address) |
                   pl2_count * page_size * pl3_index(address) |
                   page_size * pl2_index(address));
}

static inline bool is_pl3_present(struct page_table *page_table,
                                  void *address) {
    return page_table->entries[pl4_index(address)] & page_present;
}

static inline bool is_pl2_present(pl3 *level_three, void *address) {
    return (*level_three)[pl3_index(address)] & page_present;
}

static inline bool is_pl1_present(pl2 *level_two, void *address) {
    return (*level_two)[pl2_index(address)] & page_present;
}

static inline bool is_page_present(pl1 *level_one, void *address) {
    return (*level_one)[pl1_index(address)] & page_present;
}

void debug_paging(void) {
    struct page_table *page_table = current_page_table();
    for (size_t i = 0; i < pl4_count; ++i) {
        if (page_table->entries[i] * page_present) {
            logf("%i has %p\n", i, page_table->entries[i]);
        }
    }
    logf("kernel start: %p\n", &__kernel_start);
    logf("kernel end: %p\n", &__kernel_end);
}

enum status checked map_page(void *virtual_address, phys_addr phys_address,
                             enum page_attributes permissions) {

    struct page_table *page_table = current_page_table();
    logf("Mapping: %p -> %p\n", virtual_address, phys_address);
    if (!is_pl3_present(page_table, virtual_address)) {
        phys_addr phys_address = physical_alloc(1);
        page_table->entries[pl4_index(virtual_address)] =
            (phys_address | permissions | page_user_access |
             page_present);
        invalidate_tlb();
    }

    pl3 *level_three = get_pl3(virtual_address);
    if (!is_pl2_present(level_three, virtual_address)) {
        phys_addr phys_address = physical_alloc(1);
        (*level_three)[pl3_index(virtual_address)] =
            (phys_address | permissions | page_user_access |
             page_present);
        invalidate_tlb();
    }

    pl2 *level_two = get_pl2(virtual_address);
    if (!is_pl1_present(level_two, virtual_address)) {
        phys_addr phys_address = physical_alloc(1);
        (*level_two)[pl2_index(virtual_address)] =
            (phys_address | permissions | page_user_access |
             page_present);
        invalidate_tlb();
    }

    pl1 *level_one = get_pl1(virtual_address);
    if (is_page_present(level_one, virtual_address)) {
        logf("The virtual address %p is already present\n", virtual_address);
        return Error_Present;
    }

    (*level_one)[pl1_index(virtual_address)] = (phys_address | permissions | page_present);
    invalidate_tlb();

    int *test = virtual_address;
    logf("Testing: %p\n", test[0]);

    return Ok;
}

void unmap_page(void *address, bool free_physical_memory) {
    struct page_table *page_table = current_page_table();
    if (is_pl3_present(page_table, address) &&
        is_pl2_present(get_pl3(address), address) &&
        is_pl1_present(get_pl2(address), address)) {
        pl1 *level_one = get_pl1(address);


        if (free_physical_memory) {
            physical_free(align_page(*level_one[pl1_index(address)]), 1);
        }
        *level_one[pl1_index(address)] = free_page_entry;
        invalidate_page(address);
    }
}

enum status map_external_page(struct page_table *page_table,
                              void *virtual_address, phys_addr phys_address, enum page_attributes
                              permissions) {
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

/*
   void init_page_table(struct page_table *page_table) {
    assert(0);
   }
 */
