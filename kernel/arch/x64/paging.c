#include <arch/x64/control_registers.h>
#include <arch/x64/paging.h>
#include <truth/heap.h>
#include <truth/log.h>
#include <truth/physical_allocator.h>
#include <truth/panic.h>
#include <truth/slab.h>
#include <truth/string.h>
#include <truth/types.h>

extern void invalidate_tlb(void);

static void paging_page_invalidate(void *virt) {
    __asm__ volatile ("invlpg %0" ::"m"(*(uint8_t *)virt));
}

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


static bool paging_test(void) {
    size_t orig_usage = heap_get_usage();

    struct page_table original_pt = { .physical_address = read_cr3(), };
    struct page_table *new_pt = page_table_init();
    page_table_switch(new_pt->physical_address);

    assert_ok(map_page(NULL, physical_alloc(), Memory_Writable));
    int *test = (int *)0x20;
    *test = 10;
    assert(*test == 10);
    unmap_page(NULL, true);

    page_table_switch(original_pt.physical_address);
    page_table_fini(new_pt);

    size_t final_usage = heap_get_usage();
    assert(orig_usage == final_usage);

    log(Log_Debug, "Paging test passed");
    return true;
}

phys_addr page_entry_to_phys(uint64_t entry) {
    return entry & Phys_Addr_Mask;
}

uint64_t page_entry_clone(phys_addr new_entry, uint64_t old_entry) {
    return (old_entry & ~Phys_Addr_Mask) | new_entry;
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

static pl4_entry *get_pl4(void) {
    return (pl4_entry *)01777774004004004000000;
}

static pl3_entry *get_pl3_index(size_t pl4_index) {
    return (pl3_entry *)(01777774004004000000000 | (pl4_index << 12));
}

static pl2_entry *get_pl2_index(size_t pl4_index, size_t pl3_index) {
    return (pl2_entry *)(01777774004000000000000 |
                   (pl4_index << 21) |
                   (pl3_index << 12));
}

static pl1_entry *get_pl1_index(size_t pl4_index, size_t pl3_index,
                          size_t pl2_index) {
    return (pl1_entry *)(01777774000000000000000 |
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

static inline bool is_pl3_present(pl4_entry *pl4,
                                  void *address) {
    return (pl4[pl4_index(address)] & Memory_Present) == 1;
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

void *virt_from_indices(size_t i4, size_t i3, size_t i2, size_t i1) {
    return (void *)((i4 << pl4_offset) |
           (i3 << pl3_offset) |
           (i2 << pl2_offset) |
           (i1 << pl1_offset));
}

void debug_paging(void) {
    pl4_entry *pl4 = get_pl4();
    for (size_t i = 0; i < pl4_Count; ++i) {
        if (pl4[i] * Memory_Present) {
            logf(Log_Debug, "%zu has %lx\n", i, pl4[i]);
        }
    }
    logf(Log_Debug, "kernel start: %p\n", Kernel_Virtual_Start);
    logf(Log_Debug, "kernel end: %p\n", Kernel_Virtual_End);
}

bool debug_me = false;

enum status checked map_page(void *virtual_address, phys_addr phys_address,
                             enum memory_attributes permissions) {

    if ((permissions & Memory_Writable) && !(permissions & Memory_No_Execute))
    {
        return Error_Permissions;
    } else if (!memory_is_lower_half(virtual_address) &&
        (permissions & Memory_User_Access)) {
        return Error_Permissions;
    }

    pl4_entry *pl4 = get_pl4();
    pl3_entry *level_three = get_pl3(virtual_address);
    pl2_entry *level_two = get_pl2(virtual_address);
    pl1_entry *level_one = get_pl1(virtual_address);

    if (!is_pl3_present(pl4, virtual_address)) {
        phys_addr phys_address = physical_alloc();
        if (phys_address == invalid_phys_addr) {
            return Error_No_Memory;
        }
        pl4[pl4_index(virtual_address)] =
            (phys_address | (permissions & Memory_Execute_Mask) | Memory_User_Access |
             Memory_Present);
        paging_page_invalidate(level_three);
    }

    if (!is_pl2_present(level_three, virtual_address)) {
        phys_addr phys_address = physical_alloc();
        if (phys_address == invalid_phys_addr) {
            return Error_No_Memory;
        }
        level_three[pl3_index(virtual_address)] =
            (phys_address | (permissions & Memory_Execute_Mask) | Memory_User_Access |
             Memory_Present);
        paging_page_invalidate(level_two);
    }

    if (!is_pl1_present(level_two, virtual_address)) {
        phys_addr phys_address = physical_alloc();
        if (phys_address == invalid_phys_addr) {
            return Error_No_Memory;
        }
        level_two[pl2_index(virtual_address)] =
            (phys_address | (permissions & Memory_Execute_Mask) | Memory_User_Access |
             Memory_Present);
        paging_page_invalidate(level_one);
    }

    if (is_Memory_Present(level_one, virtual_address)) {
        logf(Log_Debug, "The virtual address %p is already present\n",
             virtual_address);
        return Error_Present;
    }

    level_one[pl1_index(virtual_address)] =
        (phys_address | permissions | Memory_Present);
    paging_page_invalidate(virtual_address);

    return Ok;
}

void unmap_page(void *address, bool free_physical_memory) {
    pl4_entry *pl4 = get_pl4();
    if (is_pl3_present(pl4, address) &&
        is_pl2_present(get_pl3(address), address) &&
        is_pl1_present(get_pl2(address), address)) {
        pl1_entry *level_one = get_pl1(address);


        if (free_physical_memory) {
            physical_free(align_page(level_one[pl1_index(address)]));
        }
        level_one[pl1_index(address)] = free_page_entry;
        paging_page_invalidate(address);
    }
}

enum status map_external_page(struct page_table *page_table,
                              void *virtual_address, phys_addr phys_address,
                              enum memory_attributes permissions) {
    phys_addr original_paging = read_cr3();
    write_cr3(page_table->physical_address);
    enum status status = map_page(virtual_address, phys_address, permissions);
    write_cr3(original_paging);
    return status;
}

void unmap_external_page(struct page_table *page_table, void *virtual_address,
                         bool free_physical_memory) {
    phys_addr original_paging = read_cr3();
    write_cr3(page_table->physical_address);
    unmap_page(virtual_address, free_physical_memory);
    write_cr3(original_paging);
}

void unmap_range(void *virt, size_t count, bool free_phys) {
    for (size_t i = 0; i < count; ++i) {
        unmap_page(virt, free_phys);
        virt += Page_Small;
    }
}

enum status map_range(void *virt, phys_addr phys, size_t count,
                      enum memory_attributes attrs) {
    enum status status;
    void *original_virt = virt;
    for (size_t i = 0; i < count; ++i) {
        status = map_page(virt, phys, attrs);
        if (status != Ok) {
            unmap_range(original_virt, i, false);
            return Error_Present;
        }
        virt += Page_Small;
        phys += Page_Small;
    }
    return Ok;
}

void page_table_switch(phys_addr physical_address) {
    write_cr3(physical_address);
}

enum status paging_init(void) {
    pl4_entry *pl4 = get_pl4();
    pl3_entry *pl3 = get_pl3_index(0);
    for (size_t i = pl4_Count / 2; i < pl4_Count; ++i) {
        if (!(pl4[i] & Memory_Present)) {
            phys_addr new_phys;
            void *virt = slab_alloc_phys(&new_phys, Memory_Writable);
            if (virt == NULL) {
                return Error_No_Memory;
            }
            memset(virt, 0, Page_Small);
            slab_free_virt(Page_Small, virt);
            pl4[i] = new_phys | Memory_Present | Memory_Writable;
        }
    }
    pl3[0] = 0;
    pl4[0] = 0;
    invalidate_tlb();
    assert(paging_test() == true);
    return Ok;
}

struct page_table *page_table_init(void) {
    struct page_table *pt = kmalloc(sizeof(struct page_table));
    pl4_entry *pl4 = slab_alloc_phys(&pt->physical_address,
            Memory_Writable);
    if (pl4 == NULL) {
        return NULL;
    }
    memset(pl4, 0, Page_Small / 2);
    memcpy(&pl4[pl4_Count / 2], &get_pl4()[pl4_Count / 2], Page_Small / 2);
    pl4[Kernel_Fractal_Page_Table_Index] = pt->physical_address | Memory_Writable |
                         Memory_Present;
    slab_free_virt(Page_Small, pl4);
    return pt;
}

// Don't bother freeing virtual addresses -- the address space is going away
// too.
void page_table_fini(struct page_table *pt) {
    phys_addr original_pt = read_cr3();
    page_table_switch(pt->physical_address);
    for (size_t i4 = 0; i4 < pl4_Count / 2; ++i4) {
        pl4_entry *level_four = get_pl4();
        if (level_four[i4] & Memory_Present) {
            pl3_entry *level_three = get_pl3_index(i4);
            for (size_t i3 = 0; i3 < pl3_Count; ++i3) {
                if (level_three[i3] & Memory_Present) {
                    pl2_entry *level_two = get_pl2_index(i4, i3);
                    for (size_t i2 = 0; i2 < pl2_Count; ++i2) {
                        if (level_two[i2] & Memory_Present) {
                            pl1_entry *level_one = get_pl1_index(i4, i3, i2);
                            for (size_t i1 = 0; i1 < pl1_Count; ++i1) {
                                if (level_one[i1] & Memory_Present) {
                                    physical_free(page_entry_to_phys(
                                                    level_one[i1]));
                                }
                            }
                            physical_free(page_entry_to_phys(level_two[i2]));
                        }
                    }
                    physical_free(page_entry_to_phys(level_three[i3]));
                }
            }
            physical_free(page_entry_to_phys(level_four[i4]));
        }
    }
    page_table_switch(original_pt);
    physical_free(pt->physical_address);
    kfree(pt);
}


// FIXME: Implement COW
struct page_table *page_table_clone(struct page_table *pt) {
    pl4_entry *level_four_clone = NULL;
    pl3_entry *level_three_clone = NULL;
    pl2_entry *level_two_clone = NULL;
    pl1_entry *level_one_clone = NULL;
    phys_addr new_phys;
    uint64_t *original_page;
    phys_addr original_phys = read_cr3();
    page_table_switch(pt->physical_address);
    struct page_table *pt_clone = kmalloc(sizeof(struct page_table));
    if (pt_clone == NULL) {
        goto err;
    }
    level_four_clone = slab_alloc_phys(&new_phys, Memory_Writable | Memory_User_Access);
    if (level_four_clone == NULL) {
        goto err;
    }
    pt_clone->physical_address = new_phys;
    pl4_entry *level_four = get_pl4();
    memcpy(&level_four_clone[Page_Small / 2], get_pl4(), pl4_Count / 2);
    for (size_t i4 = 0; i4 < pl4_Count / 2; ++i4) {
        if (level_four_clone[i4] & Memory_Present) {

            phys_addr l3_clone_phys;
            pl3_entry *level_three_clone = slab_alloc_phys(&l3_clone_phys, Memory_Writable | Memory_User_Access);
            if (level_three_clone == NULL) {
                goto err;
            }
            level_four_clone[i4] = page_entry_clone(l3_clone_phys, level_four[i4]);
            pl3_entry *level_three = get_pl3_index(i4);

            for (size_t i3 = 0; i3 < pl3_Count; ++i3) {
                if (level_three[i3] & Memory_Present) {

                    phys_addr l2_clone_phys;
                    pl2_entry *level_two_clone = slab_alloc_phys(&l2_clone_phys, Memory_Writable | Memory_User_Access);
                    if (level_two_clone == NULL) {
                        goto err;
                    }

                    level_three_clone[i3] = page_entry_clone(l2_clone_phys, level_three[i3]);
                    pl2_entry *level_two = get_pl2_index(i4, i3);

                    for (size_t i2 = 0; i2 < pl2_Count; ++i2) {
                        if (level_two[i2] & Memory_Present) {

                            phys_addr l1_clone_phys;
                            pl1_entry *level_one_clone = slab_alloc_phys(&l1_clone_phys, Memory_Writable | Memory_User_Access);
                            if (level_one_clone == NULL) {
                                goto err;
                            }
                            level_two_clone[i2] = page_entry_clone(l1_clone_phys, level_two[i2]);
                            pl1_entry *level_one = get_pl1_index(i4, i3, i2);

                            for (size_t i1 = 0; i1 < pl1_Count; ++i1) {
                                if (level_one[i1] & Memory_Present) {
                                    phys_addr page_clone_phys;
                                    uint64_t *page_clone = slab_alloc_phys(&page_clone_phys, Memory_Writable | Memory_User_Access);
                                    if (page_clone == NULL) {
                                        goto err;
                                    }
                                    level_one_clone[i1] = page_entry_clone(page_clone_phys, level_one[i1]);
                                    original_page = virt_from_indices(i4, i3, i2, i1);
                                    memcpy(page_clone, original_page, Page_Small);
                                    slab_free_virt(Page_Small, page_clone);
                                    original_page = NULL;
                                }
                            }
                            slab_free_virt(Page_Small, level_one_clone);
                            level_two_clone = NULL;
                        }
                    }
                    slab_free_virt(Page_Small, level_two_clone);
                    level_two_clone = NULL;
                }
            }
            slab_free_virt(Page_Small, level_three_clone);
            level_three_clone = NULL;
        }
    }
    slab_free_virt(Page_Small, level_four_clone);
    page_table_switch(original_phys);
    return pt_clone;

err:
    page_table_switch(original_phys);
    page_table_fini(pt_clone);

    slab_free_virt(Page_Small, level_four_clone);
    slab_free_virt(Page_Small, level_three_clone);
    slab_free_virt(Page_Small, level_two_clone);
    slab_free_virt(Page_Small, level_one_clone);

    return NULL;
}
