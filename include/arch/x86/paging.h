#ifndef PAGING_H
#define PAGING_H
#include <stdint.h>
#include <stdbool.h>

#include <string.h>

#include <libk/kassert.h>
#include <libk/kabort.h>
#include <libk/kmem.h>


#include <arch/x86/memlayout.h>
#include <arch/x86/memlayout.h>

#define PAGE_TABLE_SIZE 1024
#define PAGE_SIZE 4096

#define PAGE_DIR_SHIFT 12
#define PAGE_TABLE_SHIFT 12

#define PAGE_DIR_INDEX(virtual_address) ((uint32_t)(virtual_address) >> PAGE_DIR_SHIFT & 0x3ff)
#define PAGE_TABLE_INDEX(virtual_address) ((uint32_t)(virtual_address) >> PAGE_TABLE_SHIFT & 0x3ff)



struct page_dir_entry {
    uint32_t present:1;
    uint32_t rw:1;
    uint32_t super_user:1;
    uint32_t write_cache:1;
    uint32_t disable_cache:1;
    uint32_t accessed:1;
    uint32_t zero:1;
    uint32_t size:1; // Set to 0 for 4kb
    uint32_t unused:4;
    uint32_t address:20;
}__attribute__((packed));

struct page_entry {
    uint32_t present:1;
    uint32_t rw:1;
    uint32_t super_user:1;
    uint32_t write_cache:1;
    uint32_t disable_cache:1;
    uint32_t accessed:1;
    uint32_t dirty:1;
    uint32_t unused:5;
    uint32_t address:20;
}__attribute__((packed));

struct page_dir_entry *kernel_page_table_install();

extern void flush_tlb(void);
extern void enable_paging(struct page_dir_entry *page_dir);
#endif
