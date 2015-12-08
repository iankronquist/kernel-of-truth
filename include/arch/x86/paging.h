#ifndef PAGING_H
#define PAGING_H
#include <stdint.h>
#include <stdbool.h>

#include <string.h>

#include <libk/kassert.h>
#include <libk/kabort.h>
#include <libk/kmem.h>

#include <arch/x86/memlayout.h>

#define PAGE_TABLE_SIZE 1024
#define PAGE_SIZE 4096

// The macros PAGE_DIRECTORY, PAGE_ALIGN, and NEXT_PAGE are defined in
// memlayout.h

#define TOP20(x) ((uintptr_t)(x) & 0xfffff000)
#define TOP10(x) ((uintptr_t)(x) & 0xffc00000)
#define MID10(x) ((uintptr_t)(x) & 0x003ff000)
#define LOW10(x) ((uintptr_t)(x) & 0x000003ff)
#define PAGE_TABLE_SIZE 1024

void kernel_page_table_install();

extern void enable_paging(uint32_t *page_dir);

#endif
