#include <loader/elf.h>
#include <loader/log.h>
#include <loader/halt.h>
#include <loader/string.h>
#include <loader/paging.h>
#include <truth/elf.h>
#include <truth/types.h>

#define ELF_BAD_BASE_ADDRESS (~0ul)

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


const void *boot_elf_get_section(const struct elf64_header *header, const size_t size, const char *name, size_t *section_size) {
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


enum status boot_elf_get_relocation_data(void *kernel_start, uintptr_t base, const struct elf_symbol *symbol, const struct elf_rela *rela, int64_t **pointer, int64_t *value) {
    int r_type = ELF64_R_TYPE(rela->r_info);
    switch (r_type) {
        case R_X86_64_RELATIVE:
            *pointer = kernel_start + (uintptr_t)base + rela->r_offset;
            *value = (uintptr_t)kernel_start + (uintptr_t)base + rela->r_addend;
            break;
        case R_X86_64_JUMP_SLOT:
        case R_X86_64_GLOB_DAT:
        case R_X86_64_64:
            *pointer = kernel_start + (uintptr_t)base + rela->r_offset;
            if (symbol->st_index == SHN_UNDEF) {
                boot_vga_log64("Undefined symbol");
                return Error_Invalid;
            } else if (r_type == R_X86_64_64) {
                *value = (uintptr_t)kernel_start + (uintptr_t)base + symbol->st_value + rela->r_addend;
            } else {
                *value = (uintptr_t)kernel_start + (uintptr_t)base + symbol->st_value;
            }
            break;
        default:
            boot_log_number(rela->r_info);
            return Error_Invalid;
    }
    return Ok;
}


void *boot_elf_get_dynamic_symbol(void *kernel_start, size_t kernel_size, const char *name, size_t name_size) {
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
        return NULL;
    }

    rela = boot_elf_get_section(kernel_start, kernel_size, ".rela.dyn", &rela_size);
    if (rela == NULL) {
        boot_vga_log64("Couldn't find section .rela.dyn");
        return NULL;
    }

    dynsym = boot_elf_get_section(kernel_start, kernel_size, ".dynsym",
                              &dynsym_size);
    if (dynsym == NULL) {
        boot_vga_log64("Couldn't find section .dynsym");
        return NULL;
    }

    base = boot_elf_get_base_address(kernel_start, kernel_size);
    if (base == NULL) {
        boot_vga_log64("Bad base");
        return NULL;
    }

    for (size_t i = 0; i < rela_size / sizeof(struct elf_rela); ++i) {
        symbol = &dynsym[ELF64_R_SYM(rela[i].r_info)];
        if ((void *)(symbol + 1) > kernel_start + kernel_size) {
            boot_vga_log64("Symbol out of bounds");
            return NULL;
        }

        enum status status = boot_elf_get_relocation_data(kernel_start, (uintptr_t)base, symbol, &rela[i], &pointer, &value);
        if (status != Ok) {
            boot_vga_log64("Couldn't read symbol type");
            return NULL;
        }
        const char *symbol_name = &dynstr[symbol->st_name];
        if ((void *)symbol_name > kernel_start + kernel_size) {
            boot_vga_log64("String out of bounds");
            return NULL;
        }
        if (strncmp(symbol_name, name, name_size) == 0) {
            return pointer;
        }
    }

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

    dynsym = boot_elf_get_section(kernel_start, kernel_size, ".dynsym", &dynsym_size);
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

        enum status status = boot_elf_get_relocation_data(kernel_start, (uintptr_t)base, symbol, &rela[i], &pointer, &value);
        if (status != Ok) {
            boot_vga_log64("Couldn't read symbol type");
            return status;
        }

        *pointer = value;
    }

    return Ok;
}


enum status boot_elf_allocate_bss(void *kernel_start, phys_addr kernel_phys, size_t kernel_size, const void **bss_end) {

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


enum status boot_elf_remap_section(void *kernel_start, phys_addr kernel_phys, size_t kernel_size, const void *virtual_base, phys_addr *section_phys, size_t *section_size, const char *section_name) {
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
