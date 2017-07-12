#include <loader/allocator.h>
#include <loader/jitter.h>
#include <loader/log.h>
#include <loader/paging.h>
#include <loader/string.h>
#include <truth/boot.h>
#include <truth/types.h>
#include <truth/memory.h>
#include <truth/elf.h>
#include <external/multiboot.h>

#define ELF_BAD_BASE_ADDRESS (~0ul)

extern uint8_t _binary_build_truth_x64_elf64_start[];
extern uint8_t _binary_build_truth_x64_elf64_end[];
#define Boot_Kernel_Physical_Start ((phys_addr)_binary_build_truth_x64_elf64_start)
#define Boot_Kernel_Physical_End ((phys_addr)_binary_build_truth_x64_elf64_end)

void boot_halt(void);
struct boot_info *Boot_Info_Pointer = NULL;





struct tss_entry {
    uint32_t reserved0;
    uint64_t stack0;
    uint64_t stack1;
    uint64_t stack2;
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} pack;

struct tss_entry Boot_TSS = {
    .reserved0 = 0,
    .stack0 = 0,
    .stack1 = 0,
    .stack2 = 0,
    .reserved1 = 0,
    .ist = { 0, 0, 0, 0, 0, 0, 0},
    .reserved2 = 0,
    .reserved3 = 0,
    .iomap_base = 0,
};

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
};

#define Gran_64_Bit_Mode (1 << 5)
#define Gran_32_Bit_Mode (1 << 6)
#define Gran_4_KB_BLocks (1 << 7)

#define gdt_entry(base, limit, access, granularity) \
    { (limit) & 0xffff, \
      (uint16_t) ((base) >> 0  & 0xffff), \
      (uint8_t)  ((base) >> 16 & 0xff), \
      (access) & 0xff, \
      ((limit) >> 16 & 0x0f) | ((granularity) & 0xf0), \
      (uint8_t) ((base) >> 24 & 0xff), \
    }

#define gdt_entry64(base, limit, access, granularity) \
    { (limit) & 0xffff, \
      (uint16_t) ((base) >> 0  & 0xffff), \
      (uint8_t)  ((base) >> 16 & 0xff), \
      (access) & 0xff, \
      ((limit) >> 16 & 0x0f) | ((granularity) & 0xf0), \
      (uint8_t) ((base) >> 24 & 0xff), \
    }, \
    { (uint16_t) ((base) >> 32 & 0xffff), \
      (uint16_t) ((base) >> 48 & 0xffff), \
      0, \
      0, \
      0, \
      0, \
    } \

struct gdt_entry Boot_GDT[] = {
    // NULL segment
    gdt_entry(0, 0, 0, 0),

    // Kernel Code Segment.
    gdt_entry(0, 0xffffffff, 0x9a, Gran_64_Bit_Mode | Gran_4_KB_BLocks),

    // Kernel Data Segment.
    gdt_entry(0, 0xffffffff, 0x92, Gran_64_Bit_Mode | Gran_4_KB_BLocks),

    // User code and data segments would go here, but we don't need them in
    // early boot.
    // Still put dummy values here so we can reuse the same TSS segment
    // number as the kernel proper.
    {0},

    {0},

    // Task Switch Segment.
    gdt_entry64(0ull, sizeof(Boot_TSS) - 1, 0xe9, 0x00),
};

struct gdt_register {
    uint16_t limit;
    uint64_t base;
} pack;

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
static void *boot_elf_get_base_address(void *kernel_start, size_t kernel_size) {
    size_t base = ELF_BAD_BASE_ADDRESS;
    const struct elf64_header *header = kernel_start;

    const struct elf_section_header *sections = kernel_start + header->e_shoff;
    if ((void *)sections + header->e_shentsize * header->e_shnum > kernel_start + kernel_size) {
        return NULL;
    }

    for (size_t i = 0; i < header->e_shnum; ++i) {
        if (sections[i].sh_type != SHT_NULL && sections[i].sh_offset < base) {
            base = sections[i].sh_offset;
        }
    }

    if (base == ELF_BAD_BASE_ADDRESS) {
        return NULL;
    }

    return (void *)base;
}


const void *boot_elf_get_section(const struct elf64_header *header,
                                   const size_t size, const char *name,
                                   size_t *section_size) {
    const void *section;
    const uint8_t *start = (const uint8_t *)header;

    const struct elf_section_header *sections = (const void *)start +
                                                    header->e_shoff;
    if ((uint8_t *)(sections + header->e_shnum) > start + size) {
        boot_vga_log64("Section header out of bounds");
        return NULL;
    }

    const char *strtab = (const char *)header +
                            sections[header->e_shstrndx].sh_offset;
    if (strtab >= (const char *)header + size) {
        boot_vga_log64("Strtab out of bounds");
        return NULL;
    }

    for (size_t i = 0; i < header->e_shnum; ++i) {
        const char *section_name = strtab + sections[i].sh_name;
        if ((uint8_t *)section_name >= start + size) {
            continue;
        }
        if (strncmp(name, section_name, strlen(name)) == 0) {
            section = start + sections[i].sh_offset;
            if ((void *)section > (void *)start + size) {
                boot_vga_log64("Section out of bounds");
                boot_vga_log64(name);
                return NULL;
            }
            *section_size = sections[i].sh_size;
            return section;
        }
    }

    boot_vga_log64("Section not found");
    boot_vga_log64(name);
    return NULL;
}


enum status boot_elf_relocate(void *kernel_start, size_t kernel_size) {
    int64_t *pointer;
    int64_t value;
    const struct elf_symbol *symbol;
    size_t rela_size;
    const struct elf_rela *rela;
    size_t dynsym_size;
    const struct elf_symbol *dynsym;
    size_t dynstr_size;
    void *base;
    const char *dynstr = boot_elf_get_section(kernel_start, kernel_size, ".dynstr", &dynstr_size);
    if (dynstr == NULL) {
        boot_vga_log64("Couldn't find section .dynstr");
        return Error_Invalid;
    }

    rela = boot_elf_get_section(kernel_start, kernel_size, ".rela.dyn", &rela_size);
    if (rela == NULL) {
        boot_vga_log64("Couldn't find section .rela.dyn");
        return Error_Invalid;
    }

    dynsym = boot_elf_get_section(kernel_start, kernel_size, ".dynsym",
                              &dynsym_size);
    if (dynsym == NULL) {
        boot_vga_log64("Couldn't find section .dynsym");
        return Error_Invalid;
    }

    rela = boot_elf_get_section(kernel_start, kernel_size, ".rela.dyn", &rela_size);
    if (rela == NULL) {
        boot_vga_log64("Couldn't find section .rela.dyn");
        return Error_Invalid;
    }

    base = boot_elf_get_base_address(kernel_start, kernel_size);
    if (base == NULL) {
        boot_vga_log64("Bad base");
        return Error_Invalid;
    }

    for (size_t i = 0; i < rela_size / sizeof(struct elf_rela); ++i) {
        symbol = &dynsym[ELF64_R_SYM(rela[i].r_info)];
        if ((void *)(symbol + 1) > kernel_start + kernel_size) {
            boot_vga_log64("Symbol out of bounds");
            return Error_Invalid;
        }

        int r_type = ELF64_R_TYPE(rela[i].r_info);
        switch (r_type) {
            case R_X86_64_RELATIVE:
                pointer = kernel_start + (uintptr_t)base + rela[i].r_offset;
                value = (uintptr_t)kernel_start + (uintptr_t)base + rela[i].r_addend;
                *pointer = value;
                break;
            case R_X86_64_JUMP_SLOT:
            case R_X86_64_GLOB_DAT:
            case R_X86_64_64:
                pointer = kernel_start + (uintptr_t)base + rela[i].r_offset;
                if (symbol->st_index == SHN_UNDEF) {
                    boot_vga_log64("Undefined symbol");
                    return Error_Invalid;
                } else if (r_type == R_X86_64_64) {
                    value = (uintptr_t)kernel_start + (uintptr_t)base + symbol->st_value + rela[i].r_addend;
                } else {
                    value = (uintptr_t)kernel_start + (uintptr_t)base + symbol->st_value;
                }
                *pointer = value;
                break;
            default:
                boot_log_number(rela[i].r_info);
                return Error_Invalid;
        }
        const char *symbol_name = &dynstr[symbol->st_name];
        if ((void *)symbol_name > kernel_start + kernel_size) {
            boot_vga_log64("String out of bounds");
            return Error_Invalid;
        }
        if (strncmp(symbol_name, Boot_Info_Name, sizeof(Boot_Info_Name)) == 0) {
            Boot_Info_Pointer = (struct boot_info *)value;
        }
    }

    if (Boot_Info_Pointer == NULL) {
        boot_vga_log64("Couldn't find Boot Info structure");
        return Error_Absent;
    }
    return Ok;
}


static enum status boot_elf_allocate_bss(void *kernel_start, phys_addr kernel_phys, size_t kernel_size, const void **bss_end) {

    size_t bss_size;
    const void *bss = boot_elf_get_section(kernel_start, kernel_size, ".bss", &bss_size);
    if (bss == NULL) {
        boot_vga_log64("Couldn't find section .bss");
        return Error_Invalid;
    }
    size_t bss_offset = bss - kernel_start;
    *bss_end = (const void *)round_next((uintptr_t)bss + bss_size, Page_Small);

    phys_addr page = round_next(kernel_phys + bss_offset + Page_Small, Page_Small);
    const void *addr = (const void *)round_next((uintptr_t)bss + Page_Small, Page_Small);
    for (; addr < *bss_end; addr += Page_Small, page += Page_Small) {
        if (boot_map_page(addr, page, Memory_Just_Writable, true) != Ok) {
            return Error_Invalid;
        }
    }

    memset((void *)bss, 0, bss_size);

    return Ok;
}

static enum status boot_remap_section(void *kernel_start, phys_addr kernel_phys, size_t kernel_size, const void *virtual_base, phys_addr *section_phys, size_t *section_size, const char *section_name) {
    enum status status;
    const void *section = boot_elf_get_section(kernel_start, kernel_size, section_name, section_size);
    if (section == NULL) {
        boot_vga_log64("Section absent.");
        return Error_Absent;
    }
    size_t section_offset = section - kernel_start;
    *section_phys = kernel_phys + section_offset;
    phys_addr section_phys_end = round_next(*section_phys + *section_size, Page_Small);
    phys_addr page = *section_phys;
    const void *addr = virtual_base;
    for (; page < section_phys_end; page += Page_Small, addr += Page_Small) {
        status = boot_map_page(addr, page, Memory_Just_Writable, false);
        if (status != Ok) {
            boot_vga_log64("Mapping already present.");
            return status;
        }
    }

    return Ok;
}


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

    size_t symtab_size;
    phys_addr symtab_phys;
    status = boot_remap_section(random, Boot_Kernel_Physical_Start, kernel_size, kernel_end, &symtab_phys, &symtab_size, ".symtab");
    if (status != Ok) {
        boot_vga_log64("Couldn't remap symtab");
        return status;
    }
    const void *symtab_end = (const void *)round_next((uintptr_t)kernel_end + symtab_size, Page_Small);

    size_t strtab_size;
    phys_addr strtab_phys;
    status = boot_remap_section(random, Boot_Kernel_Physical_Start, kernel_size, symtab_end, &strtab_phys, &strtab_size, ".strtab");
    if (status != Ok) {
        boot_vga_log64("Couldn't remap strtab");
        return status;
    }
    const void *strtab_end = (const void *)round_next((uintptr_t)symtab_end + strtab_size, Page_Small);

    if (Boot_Info_Pointer == NULL) {
        boot_vga_log64("Boot Info pointer is NULL");
        return Error_Absent;
    }

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


enum status boot_elf_kernel_enter(void *kernel_start, size_t kernel_size) {
    void (*entrypoint)(void);
    struct elf64_header *header = kernel_start;
    void *base = boot_elf_get_base_address(kernel_start, kernel_size);

    entrypoint = kernel_start + (uintptr_t)base + header->e_entry;

    entrypoint();

    // NOT REACHED
    boot_vga_log64("Kernel main should never return!");
    boot_halt();
    return Error_Invalid;
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
        boot_vga_log64("Couldn't enter kernel");
    }
}
