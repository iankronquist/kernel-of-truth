#ifndef PAGING_H
#define PAGING_H
#include <stdint.h>
#include <stdbool.h>

#include <string.h>

#include <libk/kassert.h>
#include <libk/kabort.h>
#include <libk/klog.h>
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

#define EPHYSMEMFULL (~0)

#define CUR_PAGE_DIRECTORY_ADDR ((uint32_t*)(~0<<12))
#define PAGING_DIR_PHYS_ADDR 0xffc00000

extern void enable_paging(page_frame_t page_dir);
extern void disable_paging(void);
extern void just_enable_paging(void);
extern page_frame_t get_page_dir(void);
extern void flush_tlb(void);

page_frame_t kernel_page_table_install();

page_frame_t create_page_dir(void *link_loc, void *stack_loc,
        void(*entrypoint)(), uint16_t permissions);
void *find_free_addr(uint32_t *page_dir, page_frame_t phys_addr,
        uint16_t permissions);

void map_kernel_pages(uint32_t *page_dir);

int map_page(uint32_t *page_dir, page_frame_t physical_page,
        void *virtual_address, uint16_t permissions);

int inner_unmap_page(uint32_t *page_entries, void *virtual_address,
        bool should_free_frame);

void free_table(uint32_t *page_dir);
void inner_map_kernel_pages(uint32_t *page_dir);
int inner_map_page(uint32_t *page_dir, page_frame_t physical_page,
        void *virtual_address, uint16_t permissions);
#endif
