#include <loader/allocator.h>
#include <loader/elf.h>
#include <loader/allocator.h>
#include <loader/jitter.h>
#include <loader/log.h>
#include <loader/paging.h>
#include <loader/string.h>
#include <truth/boot.h>
#include <truth/types.h>
#include <truth/memory.h>
#include <truth/elf.h>
#include <arch/x64/segments.h>
#include <external/multiboot.h>


extern uint8_t _binary_build_truth_x64_elf64_start[];
extern uint8_t _binary_build_truth_x64_elf64_end[];
#define Boot_Kernel_Physical_Start ((phys_addr)_binary_build_truth_x64_elf64_start)
#define Boot_Kernel_Physical_End ((phys_addr)_binary_build_truth_x64_elf64_end)

struct boot_info *Boot_Info_Pointer = NULL;


struct gdt_entry Boot_GDT[] = {
    // NULL segment
    gdt_entry(0, 0, 0, 0),

    // Kernel Code Segment.
    gdt_entry(0, 0xffffffff, 0x9a, Gran_64_Bit_Mode | Gran_4_KB_Blocks),

    // Kernel Data Segment.
    gdt_entry(0, 0xffffffff, 0x92, Gran_64_Bit_Mode | Gran_4_KB_Blocks),

    // User code and data segments would go here, but we don't need them in
    // early boot.
    // Still put dummy values here so we can reuse the same TSS segment
    // number as the kernel proper.
    {0},

    {0},
    // We don't need the TSS though.
};

struct gdt_register Boot_GDT_Register = {
    .limit = sizeof(Boot_GDT) - 1,
    .base = (uint64_t)&Boot_GDT,
};


/*
uint32_t boot_crc32(void *start, size_t size) {
    uint32_t checksum = 0;
    uint32_t *chunks = start;
    for (size_t i = 0; i < size / sizeof(uint32_t); ++i) {
        __asm__ ("crc32 %0, %1" : "+r"(checksum), "=r"(chunks[i]));
    }
    boot_log_number(checksum);
    return checksum;
}
*/





enum status boot_kernel_init(void *random, struct multiboot_info *multiboot_info) {
    const size_t kernel_size = Boot_Kernel_Physical_End - Boot_Kernel_Physical_Start;
    const char elf_magic[] = ELFMAG;
    if (kernel_size < sizeof(struct elf64_header)) {
        boot_vga_log64("Kernel is way too small");
        return Error_Invalid;
    }

    /*
    if ((kernel_size % sizeof(uint32_t)) != 0) {
        boot_vga_log64("Kernel size is not a multiple of 32 bits");
        return false;
    }

    if (boot_crc32(kernel_start, kernel_size) != Kernel_CRC32) {
        boot_vga_log64("Kernel checksum failed");
        return false;
    }
    */


    // FIXME W^X/DEP
    void *addr;
    phys_addr page;
    for (addr = random, page = Boot_Kernel_Physical_Start;
            addr < random + kernel_size;
            addr += Page_Small, page += Page_Small) {
        boot_map_page(addr, page, Memory_Just_Writable, false);
    }

    struct elf64_header *header = (struct elf64_header *)Boot_Kernel_Physical_Start;
    for (size_t i = 0; i < 4; ++i) {
        if (header->e_ident[i] != elf_magic[i]) {
            boot_vga_log64("Kernel isn't a valid ELF file");
            return Error_Invalid;
        }
    }

    const void *bss_end;
    const void *kernel_end = (void *)round_next((uintptr_t)random + kernel_size, Page_Small);
    enum status status = boot_elf_allocate_bss(random, Boot_Kernel_Physical_Start, kernel_size, &bss_end);
    if (status != Ok) {
        boot_vga_log64("Couldn't allocate bss");
        return status;
    };

    status = boot_elf_relocate((void *)random, kernel_size);
    if (status != Ok) {
        boot_vga_log64("Couldn't relocate elf");
        return status;
    }

    Boot_Info_Pointer = boot_elf_get_dynamic_symbol(random, kernel_size, Boot_Info_Name, sizeof(Boot_Info_Name));
    if (Boot_Info_Pointer == NULL) {
        boot_vga_log64("Boot Info pointer is NULL");
        return Error_Absent;
    }

    size_t symtab_size;
    phys_addr symtab_phys;
    status = boot_elf_remap_section(random, Boot_Kernel_Physical_Start, kernel_size, kernel_end, &symtab_phys, &symtab_size, ".symtab");
    if (status != Ok) {
        boot_vga_log64("Couldn't remap symtab");
        return status;
    }
    const void *symtab_end = (const void *)round_next((uintptr_t)kernel_end + symtab_size, Page_Small);

    size_t strtab_size;
    phys_addr strtab_phys;
    status = boot_elf_remap_section(random, Boot_Kernel_Physical_Start, kernel_size, symtab_end, &strtab_phys, &strtab_size, ".strtab");
    if (status != Ok) {
        boot_vga_log64("Couldn't remap strtab");
        return status;
    }
    const void *strtab_end = (const void *)round_next((uintptr_t)symtab_end + strtab_size, Page_Small);

    Boot_Info_Pointer->kernel = random;
    Boot_Info_Pointer->kernel_size = kernel_size;
    Boot_Info_Pointer->kernel_physical = Boot_Kernel_Physical_Start;
    Boot_Info_Pointer->strtab_size = strtab_size;
    Boot_Info_Pointer->strtab = kernel_end;
    Boot_Info_Pointer->strtab_physical = strtab_phys;
    Boot_Info_Pointer->symtab_size = symtab_size;
    Boot_Info_Pointer->symtab = strtab_end;
    Boot_Info_Pointer->symtab_physical = symtab_phys;
    Boot_Info_Pointer->multiboot_info = multiboot_info;

    return Ok;
}


void boot_loader_main(struct multiboot_info *multiboot_info_physical) {
    uint64_t random;
    size_t kernel_size = Boot_Kernel_Physical_End - Boot_Kernel_Physical_Start;
    boot_vga_log64("Kernel of Truth Secondary Loader");
    boot_allocator_init(multiboot_info_physical->mem_lower);
    do {
        random = boot_memory_jitter_calculate() ^ Boot_Compile_Random_Number;
        random |= Memory_Kernel_Set_Mask;
        random = align_as(random, Page_Small);
    } while (~0ul - random < kernel_size);


    enum status status = boot_kernel_init((void *)random, multiboot_info_physical);
    if (status != Ok) {
        boot_vga_log64("Failed to load kernel");
        return;
    }

    status = boot_elf_kernel_enter((void *)random, kernel_size);
    if (status != Ok) {
        boot_vga_log64("Failed to enter kernel");
        return;
    }
}
