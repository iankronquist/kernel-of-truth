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

typedef uint32_t page_frame_t;
#define PAGE_FRAME_CACHE_SIZE 32
#define PAGE_FRAME_MAP_SIZE (PHYS_MEMORY_SIZE/8/PAGE_SIZE)
extern uint32_t kernel_end;


#define PAGE_ALIGN(x) (((uintptr_t)(x)) & ~0xfff)
#define NEXT_PAGE(x) (((uintptr_t)(x)+PAGE_SIZE) & ~0xfff)
#define TOP20(x) ((uintptr_t)(x) & 0xfffff000)
#define TOP10(x) ((uintptr_t)(x) & 0xffc00000)
#define MID10(x) ((uintptr_t)(x) & 0x003ff000)
#define LOW10(x) ((uintptr_t)(x) & 0x000003ff)
#define PAGE_TABLE_SIZE 1024
#define PAGE_DIRECTORY NEXT_PAGE(&kernel_end)


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

void kernel_page_table_install();
struct page_dir_entry *alloc_page_dir_entry(bool rw, bool super_user,
        bool write_cache, bool disable_cache);
void free_page_dir_entry(struct page_dir_entry *dir);

void *alloc_page(void *virtual, bool rw, bool super_user, bool write_cache,
        bool disable_cache);
void free_page(void *virtual);

extern void flush_tlb(void);
extern void enable_paging(struct page_dir_entry page_dir[1024]);

#endif
