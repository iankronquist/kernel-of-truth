#pragma once

#include <truth/memory.h>
#include <arch/x64/segments.h>

#define MB_Magic     0x1badb002
#define MB_Alignment (1 << 1)
#define MB_Info      (1 << 1)
#define MB_Flags     (MB_Alignment | MB_Info)
#define MB_Checksum  -(MB_Magic + MB_Flags)

#define CR0_Paging        0x80000000
#define CR0_Write_Protect 0x00010000

#define CR4_PAE           0x00000020

#define Page_Present         (1 << 0)
#define Page_Write           (1 << 1)
#define Page_User            (1 << 2)
#define Page_Execute_Disable (1 << 63)
#define Page_Execute_Disable_High (1 << 31)

#define IA32_EFER_MSR 0xc0000080
