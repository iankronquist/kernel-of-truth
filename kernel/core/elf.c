#include <truth/elf.h>
#include <truth/log.h>
#include <truth/string.h>
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


static const void *elf_get_section(const struct elf64_header *header,
                                   const size_t size, const char *name,
                                   size_t *section_size) {
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
            *section_size = sections[i].sh_size;
            return start + sections[i].sh_offset;
        }
    }

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
