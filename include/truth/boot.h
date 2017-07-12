#pragma once

#include <truth/types.h>
#include <external/multiboot.h>

struct boot_info {
    const void *kernel;
    phys_addr kernel_physical;
    size_t kernel_size;
    const void *strtab;
    phys_addr strtab_physical;
    size_t strtab_size;
    const void *symtab;
    phys_addr symtab_physical;
    size_t symtab_size;
    struct multiboot_info *multiboot_info;
};

#define Boot_Info_Name "Boot_Info"
