#pragma once

#include <truth/types.h>

#include <contrib/multiboot.h>

#define CUR_PAGE_DIRECTORY_ADDR ((uint32_t*)(0xfffff000))

#define PAGE_PRESENT 1
#define PAGE_WRITABLE 2
#define PAGE_USER_MODE 4
#define PAGE_WRITE_THROUGH 8
#define PAGE_CACHE_DISABLE 16
#define PAGE_ACCESSED 32
#define PAGE_DIRTY 64

// Set the current page table to the provided top level directory and enable
// paging.
extern void enable_paging(page_frame_t page_dir);
// Disable paging.
extern void disable_paging(void);
// Do not change the page table, just flip the paging bit.
extern void just_enable_paging(void);
// Get the current page table directory.
extern page_frame_t get_page_dir(void);
// Flush the translation look-aside buffer.
extern void flush_tlb(void);

// Install the kernel page table.
page_frame_t kernel_page_table_install(struct multiboot_info*);

// Create a new page directory and map important data.
// Map code at entrypoint, the code location, and the code's stack.
page_frame_t create_page_dir(void *link_loc, void *stack_loc,
        void(*entrypoint)(), uint16_t permissions);

// Map a page when paging is disabled.
// Map a physical page in the page directory at the given virtual address with
// permissions.
int map_page(uint32_t *page_dir, page_frame_t physical_page,
        void *virtual_address, uint16_t permissions);

// Unmap a page from a different page directory than the current one.
// if should_free_frame is true, the resident page frame will be marked as
// free.
int inner_unmap_page(uint32_t *page_entries, void *virtual_address,
        bool should_free_frame);

// Free a page table.
void free_table(uint32_t *page_dir);
// Map a page into a different page table than the current one.
int inner_map_page(uint32_t *page_dir, page_frame_t physical_page,
        void *virtual_address, uint16_t permissions);
