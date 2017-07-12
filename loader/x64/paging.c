#include <loader/paging.h>
#include <truth/types.h>

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

static inline uint64_t boot_get_cr3(void) {
    uint64_t cr3;
    __asm__("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

static inline size_t pl4_index(const void *address) {
    return (uintptr_t)address >> pl4_offset & pl4_mask;
}

static inline size_t pl3_index(const void *address) {
    return ((uintptr_t)address >> pl3_offset) & pl3_mask;
}

static inline size_t pl2_index(const void *address) {
    return ((uintptr_t)address >> pl2_offset) & pl2_mask;
}

static inline size_t pl1_index(const void *address) {
    return ((uintptr_t)address >> pl1_offset) & pl1_mask;
}

static void paging_page_invalidate(const void *virt) {
    __asm__ volatile ("invlpg %0" ::"m"(*(uint8_t *)virt));
}

static inline bool is_pl3_present(phys_addr *pl4, const void *address) {
    return (pl4[pl4_index(address)] & Memory_Present) == 1;
}

static inline bool is_pl2_present(phys_addr *level_three, const void *address) {
    return level_three[pl3_index(address)] & Memory_Present;
}

static inline bool is_pl1_present(phys_addr *level_two, const void *address) {
    return level_two[pl2_index(address)] & Memory_Present;
}

static inline bool is_Memory_Present(phys_addr *level_one, const void *address) {
    return level_one[pl1_index(address)] & Memory_Present;
}

static phys_addr *boot_page_table_entry(phys_addr p) {
    return (phys_addr *)(p & ~Memory_Permissions_Mask);
}

enum status boot_map_page(const void *virtual_address, phys_addr phys_address, enum memory_attributes permissions, bool force) {

    phys_address = (phys_addr)boot_page_table_entry(phys_address);
    phys_addr *level_four = (phys_addr *)boot_get_cr3();
    phys_addr *level_three;
    phys_addr *level_two;
    phys_addr *level_one;

    if (!is_pl3_present(level_four, virtual_address)) {
        phys_addr phys_address = (phys_addr)boot_allocator(Page_Small/KB);
        if (phys_address == invalid_phys_addr) {
            return Error_No_Memory;
        }
        level_four[pl4_index(virtual_address)] = phys_address | Memory_Just_Writable | Memory_User_Access | Memory_Present;
    }
    level_three = boot_page_table_entry(level_four[pl4_index(virtual_address)]);

    if (!is_pl2_present(level_three, virtual_address)) {
        phys_addr phys_address = (phys_addr)boot_allocator(Page_Small/KB);
        if (phys_address == invalid_phys_addr) {
            return Error_No_Memory;
        }
        level_three[pl3_index(virtual_address)] = phys_address | Memory_Just_Writable | Memory_User_Access | Memory_Present;
    }
    level_two = boot_page_table_entry(level_three[pl3_index(virtual_address)]);

    if (!is_pl1_present(level_two, virtual_address)) {
        phys_addr phys_address = (phys_addr)boot_allocator(Page_Small/KB);
        if (phys_address == invalid_phys_addr) {
            return Error_No_Memory;
        }
        level_two[pl2_index(virtual_address)] = phys_address | Memory_Just_Writable | Memory_User_Access | Memory_Present;
    }
    level_one = boot_page_table_entry(level_two[pl2_index(virtual_address)]);


    if (!force && is_Memory_Present(level_one, virtual_address)) {
        boot_vga_log64("The virtual address is already present");
        boot_log_number((uintptr_t)virtual_address);
        return Error_Present;
    }

    level_one[pl1_index(virtual_address)] = (phys_address | permissions | Memory_Present);
    paging_page_invalidate(virtual_address);

    return Ok;
}

