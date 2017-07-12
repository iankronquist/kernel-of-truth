#include <truth/elf.h>
#include <truth/types.h>

const void *boot_elf_get_section(const struct elf64_header *header, const size_t size, const char *name, size_t *section_size);
enum status boot_elf_relocate(void *kernel_start, size_t kernel_size);
enum status boot_elf_allocate_bss(void *kernel_start, phys_addr kernel_phys, size_t kernel_size, const void **bss_end);
enum status boot_elf_remap_section(void *kernel_start, phys_addr kernel_phys, size_t kernel_size, const void *virtual_base, phys_addr *section_phys, size_t *section_size, const char *section_name);
enum status boot_elf_kernel_enter(void *kernel_start, size_t kernel_size);
void *boot_elf_get_dynamic_symbol(void *kernel_start, size_t kernel_size, const char *name, size_t name_size);
