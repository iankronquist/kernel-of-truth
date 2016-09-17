#pragma once

#define MB_MAGIC    0x1badb002
#define MB_ALIGN    (1 << 1)
#define MB_INFO     (1 << 1)
#define MB_FLAGS    (MB_ALIGN | MB_INFO)
#define MB_CHECKSUM -(MB_MAGIC + MB_FLAGS)

#define KB 0x400
#define MB 0x100000
#define SMALL_PAGE 0x1000

#define PL1_SIZE 0x1000
#define PL2_SIZE 0x1000
#define PL3_SIZE 0x1000
#define PL4_SIZE 0x1000

#define NULL_SEGMENT      0x0
#define CODE_SEGMENT      0x8
#define DATA_SEGMENT      0x10
#define USER_CODE_SEGMENT 0x18
#define USER_DATA_SEGMENT 0x20
#define TSS_SEGMENT       0x28
#define RPL               0x3

#define CR0_PAGING        0x80010000
#define CR0_WRITE_PROTECT 0x00010000

#define CR4_PAE           0x00000020

#define PAGE_PRESENT      (1 << 0)
#define PAGE_WRITE        (1 << 1)
#define PAGE_USER         (1 << 2)
#define PAGE_EXEC_DISABLE (1 << 63)

#define IA32_EFER_MSR 0xc0000080

#define KERNEL_PAGES_COUNT ((__kernel_end - __kernel_start) / SMALL_PAGE)
