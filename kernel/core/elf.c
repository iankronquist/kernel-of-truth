#include <truth/elf.h>
#include <truth/log.h>
#include <truth/string.h>
#include <truth/symbols.h>
#include <truth/types.h>


static bool elf_verify_x86_64_compatible(const struct elf64_header *header) {
    return header->e_machine == EM_X86_64 &&
           header->e_ident[EI_CLASS] == ELFCLASS64;
}


static bool elf_verify_shared_object(const struct elf64_header *header) {
    return header->e_type == ET_REL;
}


bool elf_verify(const void *start, size_t size) {
    if (size <= sizeof(struct elf64_header)) {
        log(Log_Error, "size");
        return false;
    }

    const struct elf64_header *header = start;
    if (memcmp(header->e_ident, ELFMAG, 4) != 0) {
        log(Log_Error, "ELF Bad Magic");
        return false;
    }

    if (header->e_version != EV_CURRENT) {
        log(Log_Error, "ELF Bad Version");
        return false;
    }

    if (!elf_verify_x86_64_compatible(header)) {
        log(Log_Error, "ELF Not x86_64 Compatible");
        return false;
    }

    if (header->e_shstrndx > header->e_shnum) {
        log(Log_Error, "ELF Bad Section Header String Table Index");
        return false;
    }


    return true;
}


const char *elf_strtab_get_symbol(struct elf64_header *header, size_t size,
                            size_t index) {
    struct elf_section_header *sections = (void *)header + header->e_shoff;
    struct elf_section_header *strtab_header = &sections[header->e_shstrndx];
    const char *strtab = (void *)header + strtab_header->sh_offset;
    if (((void *)strtab > (void *)header + size) ||
        ((void *)strtab + index > (void *)header + size)) {
        log(Log_Error, "Strtab out of bounds");
        return NULL;
    }
    return strtab + index;
}


void elf_info(struct elf64_header *header) {
    logf(Log_Debug, "ELF File:\n");
    logf(Log_Debug, "e_ident         %hhx%c%c%c\n",
         header->e_ident[0],
         header->e_ident[1],
         header->e_ident[2],
         header->e_ident[3]);
    logf(Log_Debug, "e_type          %x\n", header->e_type);
    logf(Log_Debug, "e_machine       %x\n", header->e_machine);
    logf(Log_Debug, "e_version       %x\n", header->e_version);
    logf(Log_Debug, "e_entry         %lx\n", header->e_entry);
    logf(Log_Debug, "e_phoff         %lx\n", header->e_phoff);
    logf(Log_Debug, "e_shoff         %lx\n", header->e_shoff);
    logf(Log_Debug, "e_flags         %x\n", header->e_flags);
    logf(Log_Debug, "e_ehsize        %x\n", header->e_ehsize);
    logf(Log_Debug, "e_phentsize     %x\n", header->e_phentsize);
    logf(Log_Debug, "e_phnum         %x\n", header->e_phnum);
    logf(Log_Debug, "e_shentsize     %x\n", header->e_shentsize);
    logf(Log_Debug, "e_shnum         %x\n", header->e_shnum);
    logf(Log_Debug, "e_shstrndx      %x\n", header->e_shstrndx);
}


void elf_print_sections(struct elf64_header *header, size_t size) {
    struct elf_section_header *sections = (void *)((uint8_t *)header + header->e_shoff);
    struct elf_section_header *section_last = sections + header->e_shnum;
    if ((uint8_t *)section_last > (uint8_t *)header + size ||
            section_last < sections) {
        log(Log_Error, "Section out of bounds");
        return;
    }
    char *strtab = (char *)header + sections[header->e_shstrndx].sh_offset;
    if (strtab >= (char *)header + size) {
        log(Log_Error, "Strtab out of bounds\n");
    }

    for (size_t i = 0; i < header->e_shnum; ++i) {
        logf(Log_Debug, "%s\n", &strtab[sections[i].sh_name]);
        logf(Log_Debug, "%zu sh_name offset %x sh_type %x sh_offset %lx\n",
             i, sections[i].sh_name, sections[i].sh_type,
             sections[i].sh_offset);
    }
}


const struct elf_section_header *elf_get_section_index(
                                   const struct elf64_header *header,
                                   const size_t size, size_t index) {
    const uint8_t *start = (const uint8_t *)header;

    const struct elf_section_header *sections = (const void *)start +
                                                    header->e_shoff;
    if ((uint8_t *)(sections + header->e_shnum) > start + size) {
        log(Log_Error, "Section header out of bounds");
        return NULL;
    }

    if ((void *)&sections[index] < (void *)start + size) {
        return &sections[index];
    } else {
        logf(Log_Error, "Section %zu out of bounds\n", index);
        return NULL;
    }
}


const void *elf_get_section(const struct elf64_header *header,
                                   const size_t size, const char *name,
                                   size_t *section_size) {
    const void *section;
    const uint8_t *start = (const uint8_t *)header;

    const struct elf_section_header *sections = (const void *)start +
                                                    header->e_shoff;
    if ((uint8_t *)(sections + header->e_shnum) > start + size) {
        log(Log_Error, "Section header out of bounds");
        return NULL;
    }

    const char *strtab = (const char *)header +
                            sections[header->e_shstrndx].sh_offset;
    if (strtab >= (const char *)header + size) {
        log(Log_Error, "Strtab out of bounds");
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
                logf(Log_Info, "Section %s out of bounds\n", name);
                return NULL;
            }
            *section_size = sections[i].sh_size;
            return section;
        }
    }

    logf(Log_Info, "Section %s not found\n", name);
    return NULL;
}


const char *elf_get_shared_object_name(const struct elf64_header *header,
                                       const size_t size) {
    const uint8_t *start = (const uint8_t *)header;
    if (!elf_verify(header, size) || elf_verify_shared_object(header)) {
        return NULL;
    }

    size_t dynamic_size;
    const struct elf_dyn *dynamic = elf_get_section(header, size, ".dynamic",
                                                    &dynamic_size);
    if (dynamic == NULL || (uint8_t *)dynamic + dynamic_size > start + size) {
        return NULL;
    }

    size_t dynstrtab_size;
    const char *dynstrtab = elf_get_section(header, size, ".dynstr",
                                            &dynstrtab_size);
    if (dynstrtab == NULL ||
        (uint8_t *)dynstrtab + dynstrtab_size > start + size) {
        return NULL;
    }

    for (size_t i = 0; i < dynamic_size / sizeof(struct elf_dyn); ++i) {
        if (dynamic[i].d_tag == DT_SONAME) {
            const char *soname = dynstrtab + dynamic[i].d_un.d_val;
            if (soname > dynstrtab + dynstrtab_size) {
                return NULL;
            }
            return soname;
        }
    }

    return NULL;
}


enum status elf_relocate(void *module_start, size_t module_size) {
    int64_t *pointer;
    int64_t value;
    const struct elf_symbol *symbol;
    size_t rela_size;
    const struct elf_rela *rela;
    size_t dynsym_size;
    const struct elf_symbol *dynsym;
    size_t dynstr_size;
    const char *dynstr = elf_get_section(module_start,
                                         module_size,
                                         ".dynstr",
                                         &dynstr_size);
    if (dynstr == NULL) {
        log(Log_Error, "Couldn't find section .dynstr");
        return Error_Invalid;
    }

    rela = elf_get_section(module_start, module_size, ".rela.dyn", &rela_size);
    if (rela == NULL) {
        log(Log_Error, "Couldn't find section .rela.dyn");
        return Error_Invalid;
    }

    dynsym = elf_get_section(module_start, module_size, ".dynsym",
                              &dynsym_size);
    if (dynsym == NULL) {
        log(Log_Error, "Couldn't find section .dynsym");
        return Error_Invalid;
    }

    rela = elf_get_section(module_start, module_size, ".rela.dyn", &rela_size);
    if (rela == NULL) {
        log(Log_Error, "Couldn't find section .rela.dyn");
        return Error_Invalid;
    }

    logf(Log_Debug, "Rela size %lx\n", rela_size);
    for (size_t i = 0; i < rela_size / sizeof(struct elf_rela); ++i) {
        logf(Log_Debug, "Rela %lx\n", rela[i].r_info);
        int r_type = ELF64_R_TYPE(rela[i].r_info);
        switch (r_type) {
            case R_X86_64_RELATIVE:
                pointer = module_start + rela[i].r_offset;
                value = (uintptr_t)module_start + rela[i].r_addend;
                *pointer = value;
                break;
            case R_X86_64_JUMP_SLOT:
            case R_X86_64_GLOB_DAT:
            case R_X86_64_64:
                symbol = &dynsym[ELF64_R_SYM(rela[i].r_info)];
                if ((void *)(symbol + 1) > module_start + module_size) {
                    logf(Log_Error, "Symbol out of bounds %lx %p %p %p %p", ELF64_R_SYM(rela[i].r_info), symbol, module_start, module_start + module_size, dynsym);
                    return Error_Invalid;
                }
                logf(Log_Info, "Resloving rela %s\n", &dynstr[symbol->st_name]);
                pointer = module_start + rela[i].r_offset;
                if (symbol->st_index == SHN_UNDEF) {
                    if ((void *)&dynsym[symbol->st_name] > module_start + module_size) {
                        log(Log_Error, "String out of bounds");
                        return Error_Invalid;
                    }

                    value = (uintptr_t)symbol_get(&dynstr[symbol->st_name]);
                } else if (r_type == R_X86_64_64) {
                    value = (uintptr_t)module_start + symbol->st_value +
                                rela[i].r_addend;
                } else {
                    value = (uintptr_t)module_start + symbol->st_value;
                }
                *pointer = value;
                break;
            default:
                logf(Log_Error, "Unable to resolve rela symbol of type %lx\n",
                     rela[i].r_info);
                return Error_Invalid;
        }
    }

    return Ok;
}


static enum status elf_run_init_fini_helper(void *module_start, size_t module_size, int dt_array, int dt_array_size) {
    size_t funcs_size = 0;
    bool funcs_size_found = false;
    enum status (**funcs)(void) = NULL;
    size_t dynamic_size;
    const struct elf_dyn *dynamic = elf_get_section(module_start, module_size, ".dynamic", &dynamic_size);
    if (dynamic == NULL) {
        log(Log_Error, "Couldn't find section .dynamic\n");
        return Error_Invalid;
    }

    for (size_t i = 0; i < dynamic_size / sizeof(struct elf_dyn); ++i) {
        logf(Log_Info, "dynamic symbol %lx\n", dynamic[i].d_tag);
        if (dynamic[i].d_tag == dt_array) {
            funcs = module_start + dynamic[i].d_un.d_ptr;
        } else if (dynamic[i].d_tag == dt_array_size) {
            funcs_size = dynamic[i].d_un.d_val;
            funcs_size_found = true;
        }

        if (funcs != NULL && funcs_size_found) {
            break;
        }
    }

    if (funcs == NULL || !funcs_size_found) {
        log(Log_Info, "Module has no init/fini");
        return Ok;
    } else if ((void *)funcs + funcs_size > module_start + module_size) {
        log(Log_Info, "Module init/fini out of bounds");
        return Error_Invalid;
    }

    for (size_t i = 0; i < funcs_size; ++i) {
        logf(Log_Info, "Calling function %p %p\n", &funcs[i], funcs[i]);
        enum status status = funcs[i]();
        logf(Log_Info, "Called function %p %p\n", &funcs[i], funcs[i]);
        if (status != Ok) {
            return status;
        }
    }

    return Ok;
}


enum status elf_run_init(void *module_start, size_t module_size) {
    enum status status = elf_run_init_fini_helper(module_start, module_size, DT_INIT_ARRAY, DT_INIT_ARRAYSZ);
    if (status != Ok) {
        elf_run_fini(module_start, module_size);
    }
    return status;
}


enum status elf_run_fini(void *module_start, size_t module_size) {
    return elf_run_init_fini_helper(module_start, module_size, DT_FINI_ARRAY, DT_FINI_ARRAYSZ);
}



void *elf_get_symbol_address(void *elf, size_t size, const char *name) {
    const struct elf_section_header *section;
    void *location;
    size_t strtab_size;
    const char *strtab = elf_get_section(elf, size, ".strtab", &strtab_size);
    if (strtab == NULL) {
        log(Log_Error, "Could not find strtab");
        return NULL;
    }
    size_t symtab_size;
    const struct elf_symbol *symtab = elf_get_section(elf, size, ".symtab", &symtab_size);
    if (symtab == NULL) {
        log(Log_Error, "Could not find symtab");
        return NULL;
    }

    for (size_t i = 0; i < symtab_size / sizeof(struct elf_symbol); ++i) {

        if (ELF64_R_TYPE(symtab[i].st_info) != STT_FILE) {

            const char *symbol_name = &strtab[symtab[i].st_name];
            if ((void *)symbol_name > elf + size) {
                log(Log_Error, "Symbol name out of bounds");
                return NULL;
            }

            section = elf_get_section_index(elf, size, symtab[i].st_index);
            if (section == NULL) {
                log(Log_Error, "Bad symbol index");
                return NULL;
            }

            if (strncmp(symbol_name, name, strlen(name)) == 0) {
                location = elf + symtab[i].st_value;
                if (location > elf + size) {
                    log(Log_Error, "Location out of bounds");
                    return NULL;
                }
                return location;
            }
        }
    }

    return NULL;
}
