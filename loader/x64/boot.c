#include <truth/boot.h>
#include <truth/types.h>
#include <truth/memory.h>
#include <truth/elf.h>
#include <external/multiboot.h>

#define ELF_BAD_BASE_ADDRESS (~0ul)
// Memory_Writable implies NX. We need this for higher levels of the page table.
#define Memory_Just_Writable (1 << 1)
#define invalid_phys_addr 0xfff

extern uint8_t _binary_build_truth_x64_elf64_start[];
extern uint8_t _binary_build_truth_x64_elf64_end[];
#define Boot_Kernel_Physical_Start ((phys_addr)_binary_build_truth_x64_elf64_start)
#define Boot_Kernel_Physical_End ((phys_addr)_binary_build_truth_x64_elf64_end)

#define pl1_Count 512
#define pl2_Count 512
#define pl3_Count 512
#define pl4_Count 512

#define pl1_offset 12
#define pl1_mask   0777
#define pl2_offset 21
#define pl2_mask   0777
#define pl3_offset 30
#define pl3_mask   0777
#define pl4_offset 39
#define pl4_mask   0777

static inline uint64_t boot_get_cr3(void) {
    uint64_t cr3;
    __asm__("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

void boot_halt(void);
struct boot_info *Boot_Info_Pointer = NULL;


// FIXME: This is inefficient.
void *memset(void *b, int c, size_t len) {
    uint8_t *buf = b;
    for (size_t i = 0; i < len; ++i) {
        buf[i] = c;
    }
    return b;
}

size_t strlen(const char *str) {
    const char *c;
    for (c = str; *c != '\0'; ++c) { }
    return c - str;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    for (size_t i = 0; i < n && *s1 != '\0' && *s2 != '\0'; ++s1, ++s2, ++i) {
        int diff = *s1 - *s2;
        if (diff != 0) {
            return diff;
        }
    }
    return *s1 - *s2;
}


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

static void *Boot_Allocator_Next_Page = (void *)0x1000;
static void *Boot_Allocator_Last_Page = NULL;

void *boot_allocator(size_t pages) {
    void *page = Boot_Allocator_Next_Page;
    if (Boot_Allocator_Next_Page == Boot_Allocator_Last_Page) {
        return NULL;
    }

    Boot_Allocator_Next_Page += pages * Page_Small;
    return page;
}

void boot_allocator_init(uint64_t kilobytes) {
    Boot_Allocator_Last_Page = (void *)(kilobytes * KB);
}

static uint16_t *const Boot_VGA_Window = (uint16_t *)0xb8000;
static size_t Boot_VGA_Window_Index = 0;

void boot_vga_log64(const char *string) {
    for (const char *c = string; *c != '\0'; ++c) {
        Boot_VGA_Window[Boot_VGA_Window_Index] = 0x0f00 | *c;
        Boot_VGA_Window_Index++;
    }
}

void boot_log_number(uint64_t n) {
    const size_t nibbles = sizeof(uint64_t) * 2;
    char number[nibbles + 1];
    for (size_t i = 0; i < nibbles; ++i) {
        int nibble = n % 16;
        n /= 16;
        if (nibble < 10) {
            number[nibbles - i - 1] = nibble + '0';
        } else {
            number[nibbles - i - 1] = nibble + 'a' - 10;
        }
    }

    number[nibbles] = '\0';
    boot_vga_log64(" ");
    boot_vga_log64((const char *)&number);
    boot_vga_log64(" ");
}

#define Boot_Jitter_SHA1_Starting_Values 0xefcdab8967452301

#define Boot_Jitter_Max_Fold_Bits 4
#define Boot_Jitter_Buffer_Size (2 * Page_Small)
#define Boot_Jitter_Fold_Mask (0xff)

static inline uint64_t boot_cpu_get_ticks(void) {
    uint32_t eax, edx;
    __asm__ volatile ("rdtsc" : "=(eax)"(eax), "=(edx)"(edx)::);
    return (((uint64_t)edx) << 32) | eax;
}

uint64_t boot_memory_jitter_calculate(void) {
    uint8_t *memory = boot_allocator(Boot_Jitter_Buffer_Size/Page_Small);
    uint64_t entropy = Boot_Jitter_SHA1_Starting_Values;
    for (size_t i = 0; i < Boot_Jitter_Buffer_Size; ++i) {
        uint64_t before = boot_cpu_get_ticks();
        memory[i] += 1;
        uint64_t after = boot_cpu_get_ticks();
        uint64_t delta = after - before;
        entropy ^= delta & Boot_Jitter_Fold_Mask;
        entropy = (entropy << 8) | (entropy & 0xff00000000000000) >> 56;
    }
    return entropy;
}

static inline size_t pl4_index(const void *address) {
    return (uintptr_t)address >> pl4_offset & pl4_mask;
}

static inline size_t pl3_index(const void *address) {
    return ((uintptr_t)address >> pl3_offset) & pl3_mask;
}

static inline size_t pl2_index(const void *address) {
    return ((uintptr_t)address >> pl2_offset) & pl2_mask;
}

static inline size_t pl1_index(const void *address) {
    return ((uintptr_t)address >> pl1_offset) & pl1_mask;
}

static void paging_page_invalidate(const void *virt) {
    __asm__ volatile ("invlpg %0" ::"m"(*(uint8_t *)virt));
}

static phys_addr *get_pl3(const void *address) {
    return get_pl3_index(pl4_index(address));
}

static phys_addr *get_pl2(const void *address) {
    return get_pl2_index(pl4_index(address), pl3_index(address));
}

static phys_addr *get_pl1(const void *address) {
    return get_pl1_index(pl4_index(address), pl3_index(address), pl2_index(address));
}

static inline bool is_pl3_present(phys_addr *pl4, const void *address) {
    return (pl4[pl4_index(address)] & Memory_Present) == 1;
}

static inline bool is_pl2_present(phys_addr *level_three, const void *address) {
    return level_three[pl3_index(address)] & Memory_Present;
}

static inline bool is_pl1_present(phys_addr *level_two, const void *address) {
    return level_two[pl2_index(address)] & Memory_Present;
}

static inline bool is_Memory_Present(phys_addr *level_one, const void *address) {
    return level_one[pl1_index(address)] & Memory_Present;
}

phys_addr *boot_page_table_entry(phys_addr p) {
    return (phys_addr *)(p & ~Memory_Permissions_Mask);
}

enum status boot_map_page(const void *virtual_address, phys_addr phys_address, enum memory_attributes permissions, bool force) {

    phys_address = (phys_addr)boot_page_table_entry(phys_address);
    phys_addr *level_four = (phys_addr *)boot_get_cr3();
    phys_addr *level_three;
    phys_addr *level_two;
    phys_addr *level_one;

    if (!is_pl3_present(level_four, virtual_address)) {
        phys_addr phys_address = (phys_addr)boot_allocator(Page_Small/KB);
        if (phys_address == invalid_phys_addr) {
            return Error_No_Memory;
        }
        level_four[pl4_index(virtual_address)] = phys_address | Memory_Just_Writable | Memory_User_Access | Memory_Present;
    }
    level_three = boot_page_table_entry(level_four[pl4_index(virtual_address)]);

    if (!is_pl2_present(level_three, virtual_address)) {
        phys_addr phys_address = (phys_addr)boot_allocator(Page_Small/KB);
        if (phys_address == invalid_phys_addr) {
            return Error_No_Memory;
        }
        level_three[pl3_index(virtual_address)] = phys_address | Memory_Just_Writable | Memory_User_Access | Memory_Present;
    }
    level_two = boot_page_table_entry(level_three[pl3_index(virtual_address)]);

    if (!is_pl1_present(level_two, virtual_address)) {
        phys_addr phys_address = (phys_addr)boot_allocator(Page_Small/KB);
        if (phys_address == invalid_phys_addr) {
            return Error_No_Memory;
        }
        level_two[pl2_index(virtual_address)] = phys_address | Memory_Just_Writable | Memory_User_Access | Memory_Present;
    }
    level_one = boot_page_table_entry(level_two[pl2_index(virtual_address)]);


    if (!force && is_Memory_Present(level_one, virtual_address)) {
        boot_vga_log64("The virtual address is already present");
        boot_log_number((uintptr_t)virtual_address);
        return Error_Present;
    }

    level_one[pl1_index(virtual_address)] = (phys_address | permissions | Memory_Present);
    paging_page_invalidate(virtual_address);

    return Ok;
}
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
