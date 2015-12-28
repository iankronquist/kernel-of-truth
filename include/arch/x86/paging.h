#ifndef PAGING_H
#define PAGING_H
#include <stdint.h>
#include <stdbool.h>

#include <string.h>

#include <libk/kassert.h>
#include <libk/kabort.h>
#include <libk/kmem.h>
#include <libk/physical_allocator.h>

#include <arch/x86/memlayout.h>

#define PAGE_TABLE_SIZE 1024

// The macros PAGE_DIRECTORY, PAGE_ALIGN, and NEXT_PAGE are defined in
// memlayout.h

#define TOP20(x) ((uintptr_t)(x) & 0xfffff000)
#define TOP10(x) ((uintptr_t)(x) & 0xffc00000)
#define MID10(x) ((uintptr_t)(x) & 0x003ff000)
#define LOW10(x) ((uintptr_t)(x) & 0x000003ff)

#define HIGHINDEX(x) ((uintptr_t)x >> 22)
#define LOWINDEX(x) ((uintptr_t)x >> 12)
#define GETADDRESS(x) ((uintptr_t)x & ~ 0xfff)
#define GETFLAGS(x) ((uintptr_t)x & 0xfff)
#define PAGE_TABLE_SIZE 1024

#define PAGE_PRESENT 1
#define PAGE_WRITABLE 2
#define PAGE_USER_MODE 4
#define PAGE_WRITE_THROUGH 8
#define PAGE_CACHE_DISABLE 16
#define PAGE_ACCESSED 32
#define PAGE_DIRTY 64

page_frame_t *kernel_pages;

uint32_t *kernel_page_table_install();

extern void enable_paging(uint32_t *page_dir);
int map_page(uint32_t *page_dir, page_frame_t physical_page,
    void *virtual_address, uint16_t permissions);
uint32_t create_new_page_dir(page_frame_t *cur_page_dir,
    page_frame_t kernel_page, void *initial_location);
void *just_give_me_a_page(uint32_t *page_dir, page_frame_t phys_addr,
        uint16_t permissions);
void map_kernel_pages(uint32_t *page_dir);
uint32_t *get_page_entry(page_frame_t *page_dir, void *virtual_address);
void free_table(uint32_t *page_dir);
#endif
