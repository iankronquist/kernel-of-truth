#ifndef MEMLAYOUT_H
#define MEMLAYOUT_H

#include <truth/types.h>

#define KB 1024
#define MB (1024*1024)
#define GB (1024*1024*1024)

// Paging related
#define ROUND_NEXT_PAGE(x) (((x + (PAGE_SIZE / 2)) / PAGE_SIZE) * PAGE_SIZE)
#define PAGE_ALIGN(x) (((uintptr_t)(x)) & ~0xfff)
#define NEXT_PAGE(x) (((uintptr_t)(x)+PAGE_SIZE) & ~0xfff)
#define PAGE_DIRECTORY NEXT_PAGE(KERNEL_END)
#define PAGE_SIZE 4096

// FIXME: Move elsewhere
#define FRACTAL_MAP 0xffc00000

// The address of the start of the multiboot and kernel bootstrap section.
extern int bootstrap_start;
// The address of the end of the multiboot and kernel bootstrap section.
extern int bootstrap_end;
// Address of the stack canary
extern int bootstrap_stack_canary;
// The address of the start of the kernel. Defined by the linker.
extern int kernel_start;
// The address of the end of the kernel. Defined by the linker.
extern int kernel_end;

// The address of the start of the text section.
extern int text_start;
// The address of the end of the text section.
extern int text_end;
// The address of the start of the data section.
extern int data_start;
// The address of the end of the data section.
extern int data_end;
// The address of the start of the read-only data section.
extern int rodata_start;
// The address of the end of the read-only data section.
extern int rodata_end;
// The address of the start of the bss section.
extern int bss_start;
// The address of the end of the bss section.
extern int bss_end;
// The address of the start of the bootstrap heap.
extern int bootstrap_heap;

#define BOOTSTRAP_START ((uintptr_t)&bootstrap_start)
#define BOOTSTRAP_END (ROUND_NEXT_PAGE((uintptr_t)&bootstrap_end))
#define TEXT_START   ((uintptr_t)&text_start)
#define TEXT_END     (ROUND_NEXT_PAGE((uintptr_t)&text_end))
#define DATA_START   ((uintptr_t)&data_start)
#define DATA_END     (ROUND_NEXT_PAGE((uintptr_t)&data_end))
#define RODATA_START ((uintptr_t)&rodata_start)
#define RODATA_END   (ROUND_NEXT_PAGE((uintptr_t)&rodata_end))
#define BSS_START    ((uintptr_t)&bss_start)
#define BSS_END      (ROUND_NEXT_PAGE((uintptr_t)&bss_end))
#define KERNEL_START ((uintptr_t)&kernel_start)
#define KERNEL_END   (ROUND_NEXT_PAGE((uintptr_t)&kernel_end))
#define BOOTSTRAP_STACK_CANARY_START ((uintptr_t)&bootstrap_stack_canary)

#define KERNEL_SIZE (KERNEL_END - KERNEL_START)

// Heap related
#define KHEAP_PHYS_ROOT NEXT_PAGE(KERNEL_END)
#define KHEAP_PHYS_END ((void*)NEXT_PAGE(KHEAP_PHYS_ROOT))
#define KHEAP_PHYS_SIZE (size_t)(KHEAP_PHYS_END - KHEAP_PHYS_ROOT)

// FIXME: Move elsewhere
// Video memory related
#define VIDEO_MEMORY_BEGIN 0xB8000
#define VIDEO_MEMORY_SIZE (80 * 24)
#define VGA_BEGIN 0xa0000
#define VGA_END   0xbffff

#endif
