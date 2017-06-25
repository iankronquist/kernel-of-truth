#include <truth/types.h>

#define EI_CLASS 4
#define EI_NIDENT 16

struct elf64_header {
    unsigned char e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
};


#define ET_NONE 0
#define ET_REL  1
#define ET_EXEC 2
#define ET_DYN  3
#define ET_CORE 4

#define ET_LOPROC 0xff00
#define ET_HIPROC 0xffff

#define EM_NONE 0
#define EM_386  3
#define EM_860  7
#define EM_X86_64 62

#define EV_NONE    0
#define EV_CURRENT 1

#define ELFMAG "\x7f""ELF"

#define ELFCLASSNONE 0
#define ELFCLASS32 1
#define ELFCLASS64 2

#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2


#define SHN_UNDEF 0
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC    0xff00
#define SHN_HIPROC    0xff1f
#define SHN_ABS       0xfff1
#define SHN_COMMON    0xfff2
#define SHN_HIRESERVE 0xffff

struct elf_section_header {
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
} pack;

#define SHT_NULL     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_HASH     5
#define SHT_DYNAMC   6
#define SHT_NOTE     7
#define SHT_NOBITS   8
#define SHT_REL      9
#define SHT_SHLIB   10
#define SHT_DYNSYM  11
#define SHT_LOPROC  0x70000000
#define SHT_HIPROC  0x7fffffff
#define SHT_LOUSER  0x80000000
#define SHT_HIUSER  0xffffffff

#define SHF_WRITE     0x1
#define SHF_ALLOC     0x2
#define SHF_EXECINSTT 0x4
#define SHF_MASKPROC  0xf0000000

struct elf_symbol {
    uint32_t st_name;
    unsigned char st_info;
    unsigned char st_other;
    uint16_t st_index;
    uint64_t st_value;
    uint64_t st_size;
} pack;

#define ELF32_ST_BIND(i) ((i) >> 4)
#define ELF32_ST_TYPE(i) ((i) & 0x0f)
#define ELF32_ST_INFO(b, t)  (((b) << 4) + ((t) & 0x0f))

#define ELF64_ST_BIND ELF32_ST_BIND
#define ELF64_ST_TYPE ELF32_ST_TYPE
#define ELF64_ST_INFO ELF32_ST_INFO

#define STB_LOCAL  0
#define STB_GLOBAL 1
#define STB_WEAK   2
#define STB_LOPROC 13
#define STB_HIPROC 15

#define STT_NOTYPE  0
#define STT_OBJECT  1
#define STT_FUNC    2
#define STT_SECTION 3
#define STT_FILE    4
#define STT_LOPROC  13
#define STT_HIPROC  15

struct elf_rel {
    uint64_t r_offset;
    uint64_t r_info;
};

struct elf_rela {
    uint64_t r_offset;
    uint64_t r_info;
    int64_t r_addend;
};

#define ELF64_R_SYM(i)  ((i) >> 32)
#define ELF64_R_TYPE(i) ((i) & 0xffffffff)
#define ELF64_R_INFO(s, t) (((s) << 32) + (uint32_t)(t))

#define ELF32_R_SYM(i)  ((i) >> 8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s, t) (((s) << 8) + (unsigned char)(t))

#define R_X86_64_NONE      0
#define R_X86_64_64        1
#define R_X86_64_PC32      2
#define R_X86_64_GOT32     3
#define R_X86_64_PLT32     4
#define R_X86_64_COPY      5
#define R_X86_64_GLOB_DAT  6
#define R_X86_64_JUMP_SLOT 7
#define R_X86_64_RELATIVE  8
#define R_X86_64_GOTPCREL  9
#define R_X86_64_32       10
#define R_X86_64_32S      11
#define R_X86_64_16       12
#define R_X86_64_PC16     13
#define R_X86_64_8        14
#define R_X86_64_PC8      15
#define R_X86_64_NUM      16

#define ELF32_386_NONE     0
#define ELF32_386_32       1
#define ELF32_386_PC32     2
#define ELF32_386_GOT32    3
#define ELF32_386_PLT32    4
#define ELF32_386_COPY     5
#define ELF32_386_GLOB_DAT 6
#define ELF32_386_JMP_SLOT 7
#define ELF32_386_RELATIVE 8
#define ELF32_386_GOTOFF   9
#define ELF32_386_GOTPC    10

/*
#define ELF32_386_32(s, a) (s + a)
#define ELF32_386_PC32(s, a, p) (s + a - p)
#define ELF32_386_GOT32(g, a, p) (g + a - p)
#define ELF32_386_PLT32(l, a, p) (l + a - p)
#define ELF32_386_GLOB_DAT(s) (s)
#define ELF32_386_JMP_SLOT(s) (s)
#define ELF32_386_RELATIVE(b, a) (b + a)
#define ELF32_386_GOTOFF(s, a, got) (s + a - got)
#define ELF32_386_GOTPC(got, a, p) (got + a - p)
*/


struct elf_program_header {
    uint32_t p_type;
    uint32_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_flags;
    uint64_t p_align;
};

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMC 2
#define PT_INTERP 3
#define PT_NOTE   4
#define PT_SHLIB  5
#define PT_PHDR   6
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0xffffffff

struct elf_dyn {
    int64_t d_tag;
    union {
        uint64_t d_val;
        uint64_t d_ptr;
    } d_un;
};

#define DT_NULL      0
#define DT_NEEDED    1
#define DT_PLTRELSZ  2
#define DT_PLTGOT    3
#define DT_HASH      4
#define DT_STRTAB    5
#define DT_SYMTAB    6
#define DT_RELA      7
#define DT_RELASZ    8
#define DT_RELAENT   9
#define DT_STRSZ    10
#define DT_SYMENT   11
#define DT_INIT     12
#define DT_FINI     13
#define DT_SONAME   14
#define DT_RPATH    15
#define DT_SYMBOLIC 16
#define DT_REL      17
#define DT_RELSZ    18
#define DT_RELENT   19
#define DT_PLTREL   20
#define DT_TEXTREL  21
#define DT_JMPREL   22
#define DT_LOPROC   0x70000000
#define DT_HIPROC   0x7fffffff

void elf_info(struct elf64_header *header);
bool elf_verify(const void *start, size_t size);
const char *elf_strtab_get_symbol(struct elf64_header *header, size_t size,
                            size_t index);
const void *elf_get_section(const struct elf64_header *header,
                                   const size_t size, const char *name,
                                   size_t *section_size);
void elf_print_sections(struct elf64_header *header, size_t size);
const char *elf_get_shared_object_name(const struct elf64_header *header,
                                       const size_t size);

const struct elf_section_header *elf_get_section_index(
                                   const struct elf64_header *header,
                                   const size_t size, size_t index);

enum status elf_relocate(void *module_start, size_t module_size);
