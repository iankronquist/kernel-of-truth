#pragma once


#define KB 0x400
#define MB 0x100000
#define GB 0x40000000

#define Kernel_Space_Start (0xffffffff80000000)

#ifdef __ASM__
#define Page_Small  (4 * KB)
#define Page_Medium (2 * MB)
#define Page_Large  (GB)

#define Memory_Present       (1 << 0)
#define Memory_Writable      (1 << 1)
#define Memory_User_Access   (1 << 2)
#define Memory_No_Execute    (1 << 63)
#define Memory_Execute_Mask  (0x7fffffffffffffff)
#define Memory_Writable_Mask (0xfffffffffffffffd)

#define Lower_Half_Start  (Page_Small)
#define Lower_Half_End    (0x00007fffffffffff)
#define Higher_Half_Start (0xffff800000000000)
#define Higher_Half_High  (0xffff8000)
#define Higher_Half_End   (~0)
#define Lower_Half_Size   (Lower_Half_End - Lower_Half_Start)
#define Higher_Half_Size  (Higher_Half_End - Higher_Half_Start)

#define virt_to_phys(x) (x - Kernel_Space_Start)
#define phys_to_virt(x) (x + Kernel_Space_Start)


#elif __C__
#include <truth/types.h>

#define virt_to_phys(x) ((uint64_t)(x) - Kernel_Space_Start)
#define phys_to_virt(x) ((void *)((x) + Kernel_Space_Start))


enum page_size {
    Page_Small  = (4 * KB),
    Page_Medium = (2 * MB),
    Page_Large  = (GB),
};

enum memory_attributes {
    Memory_No_Attributes = (0),
    Memory_Present       = (1ul << 0),
    Memory_User_Access   = (1ul << 2),
    Memory_No_Execute    = (1ul << 63),
    Memory_Writable      = (1ul << 1) | Memory_No_Execute,
    Memory_Execute_Mask  = ~Memory_No_Execute,
};

#define Lower_Half_Start  ((void *)Page_Small)
#define Lower_Half_End    ((void *)0x00007fffffffffff)
#define Higher_Half_Start ((void *)0xffff800000000000)
#define Higher_Half_End   ((void *)~0)
#define Lower_Half_Size   (Lower_Half_End - Lower_Half_Start)
#define Higher_Half_Size  (Higher_Half_End - Higher_Half_Start)

#define Memory_Bootstrap_Stack_Size (16 * KB)

static inline bool memory_is_lower_half(void *address) {
    return address <= Lower_Half_End;
}


void memory_init(void);
void memory_user_access_enable(void);
void memory_user_access_disable(void);
#endif
