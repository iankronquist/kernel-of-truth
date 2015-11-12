#ifndef PAGING_H
#define PAGING_H
#include <stdint.h>

#define PAGE_TABLE_SIZE 1024
#define PAGE_SIZE 4096



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

struct page_entry page_table[PAGE_TABLE_SIZE];

extern void flush_tlb(void);
extern void enable_paging(uint32_t page_dir);
#endif
