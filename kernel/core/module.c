#include <truth/elf.h>
#include <truth/log.h>
#include <truth/heap.h>
#include <truth/hashtable.h>
#include <truth/memory.h>
#include <truth/panic.h>
#include <truth/slab.h>
#include <truth/symbols.h>
#include <external/multiboot.h>


struct module {
    struct module *next;
    const char *name;
    void *virtual_start;
    phys_addr physical_start;
    size_t size;
};


struct module *Module_List = NULL;


enum status module_load(void *start, size_t size) {
    elf_print_sections(start, size);
    return Error_Absent;
}


void module_list_free(void) {
    while (Module_List != NULL) {
        struct module *next = Module_List->next;
        kfree(Module_List);
        Module_List = next;
    }
}


void *module_list_offset(phys_addr mods_addr, phys_addr ptr, void *mapped) {
    return mapped + (ptr - mods_addr);
}

/*
enum status modules_kernel_symbol_hashtable_init(struct multiboot_info *info) {


    uintptr_t multiboot_elf_section_header_addr = (uintptr_t)info->u.elf_sec.addr;
    void *kernel_header = phys_to_virt(multiboot_elf_section_header_addr);
    map_page(kernel_header, multiboot_elf_section_header_addr, Memory_No_Attributes);
    logf(Log_Debug, "%x\n", info->flags);
    assert(info->flags & MULTIBOOT_INFO_ELF_SHDR);

    struct elf_section_header *kernel_symtab = NULL;
    struct elf_section_header *kernel_sections =
        phys_to_virt(info->u.elf_sec.addr);
    char *kernel_strtab = (char *)kernel_sections + kernel_sections[info->u.elf_sec.shndx].sh_offset;

    logf(Log_Debug, "%lx %lx %lx\n", info->u.elf_sec.size, info->u.elf_sec.shndx, info->u.elf_sec.num);

    for (size_t i = 0; i < info->u.elf_sec.num; ++i) {
        logf(Log_Debug, "%s\n", kernel_strtab + kernel_sections[i].sh_name);
        if (kernel_sections[i].sh_type == SHT_SYMTAB && strncmp(".strtab", kernel_strtab + kernel_sections[i].sh_name, strlen(".strtab")) == 0) {
            kernel_symtab = &kernel_sections[i];
        }
    }

    if (kernel_symtab == NULL) {
        log(Log_Debug, "Invalid kernel symbol table");
        return Error_Invalid;
    }


    return Ok;
}
*/


enum status modules_init(struct multiboot_info *info) {
    enum status modules_status = Ok;
    static int count = 0;
    assert(count == 0);
    count++;
    logf(Log_Info, "Loading %d module%s\n", info->mods_count,
         info->mods_count == 1 ? "" : "s");

    modules_status = symbol_init();
    if (modules_status != Ok) {
        return modules_status;
    }

    struct multiboot_mod_list *modules = slab_alloc_request_physical(info->mods_addr, Page_Small, Memory_No_Attributes);
    if (modules == NULL) {
        symbol_fini();
        return Error_No_Memory;
    }


    for (size_t i = 0; i < info->mods_count; ++i) {

        size_t module_size = modules[i].mod_end - modules[i].mod_start;
        void *module_start = slab_alloc_request_physical(
                    modules[i].mod_start,
                    align_as(round_next(module_size, Page_Small), Page_Small),
                    Memory_Writable);
        if (module_start == NULL) {
            modules_status = Error_Invalid;
            continue;
        }

        if (!elf_verify(module_start, module_size)) {
            slab_free(Page_Small, module_start);
            modules_status = Error_Invalid;
            continue;
        }

        char *module_name = elf_get_shared_object_name(module_start,
                                                       module_size);
        logf(Log_Info, "Loading module %s\n", module_name);

        /*
        status = elf_relocate(module_start, module_size);
        if (status != Ok) {
            module_list_free();
            slab_free(Page_Small, module_start);
            modules_status = Error_Invalid;
            continue;
        }
        */

        struct module *new_module = kmalloc(sizeof(struct module));
        if (new_module == NULL) {
            slab_free(Page_Small, module_start);
            modules_status = Error_No_Memory;
            continue;
        }
        new_module->physical_start = modules[i].mod_start;
        new_module->virtual_start = module_start;
        new_module->size = module_size;
        new_module->next = Module_List;
        new_module->name = module_name;
    }
    slab_free(Page_Small, modules);

    if (modules_status != Ok) {
        symbol_fini();
    }

    return modules_status;
}


void modules_info(struct multiboot_info *mi) {
    //enum status status;
    struct multiboot_mod_list *modules =
        (struct multiboot_mod_list *)(uintptr_t)mi->mods_addr;
    //status = map_page(modules, mi->mods_addr, Memory_No_Attributes);
    //assert_ok(status);
    logf(Log_Debug, "%p\n", modules);
    for (size_t i = 0; i < mi->mods_count; ++i) {
        logf(Log_Debug, "start %x\n", modules[i].mod_start);
        logf(Log_Debug, "end %x\n", modules[i].mod_end);
        logf(Log_Debug, "file %s\n", (char *)(uintptr_t)modules[i].cmdline);
    }
    //unmap_page(modules, false);
}
