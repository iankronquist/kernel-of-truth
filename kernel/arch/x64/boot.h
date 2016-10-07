#pragma once

#include <truth/memory_sizes.h>

#define MB_Magic     0x1badb002
#define MB_Alignment (1 << 1)
#define MB_Info      (1 << 1)
#define MB_Flags     (MB_Alignment | MB_Info)
#define MB_Checksum -(MB_Magic + MB_Flags)

#define Null_Segment      0x0
#define Code_Segment      0x8
#define Data_Segment      0x10
#define User_Code_Segment 0x18
#define User_Data_Segment 0x20
#define TSS_Segment       0x28
#define RPL               0x3

#define CR0_Paging        0x80000000
#define CR0_Write_Protect 0x00010000

#define CR4_PAE           0x00000020

#define Page_Present         (1 << 0)
#define Page_Write           (1 << 1)
#define Page_User            (1 << 2)
#define Page_Execute_Disable (1 << 63)

#define IA32_EFER_MSR 0xc0000080
