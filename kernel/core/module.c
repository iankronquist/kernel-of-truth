#include <arch/x64/paging.h>
#include <truth/crypto.h>
#include <truth/elf.h>
#include <truth/log.h>
#include <truth/key.h>
#include <truth/heap.h>
#include <truth/hashtable.h>
#include <truth/memory.h>
#include <truth/panic.h>
#include <truth/slab.h>
#include <truth/string.h>
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


void module_list_free(void) {
    while (Module_List != NULL) {
        struct module *next = Module_List->next;
        kfree(Module_List);
        Module_List = next;
    }
}


enum status checked module_verify(void *module_start, size_t module_size) {
    int error;

    size_t verify_size = module_size - crypto_sign_ed25519_BYTES - sizeof(SIGNATURE_MAGIC_STRING);
    char *signature_magic = module_start + verify_size;
    unsigned char *signature = module_start + module_size - crypto_sign_ed25519_BYTES;

    if (strncmp(signature_magic, SIGNATURE_MAGIC_STRING, sizeof(SIGNATURE_MAGIC_STRING)) != Order_Equal) {
        log(Log_Info, "Bad signature magic");
        return Error_Invalid;
    }

    error = crypto_sign_ed25519_verify_detached(signature, module_start, verify_size, Kernel_Public_Key);
    if (error != 0) {
        log(Log_Info, "Bad signature");
        return Error_Invalid;
    }

    return Ok;
}


void *module_list_offset(phys_addr mods_addr, phys_addr ptr, void *mapped) {
    return mapped + (ptr - mods_addr);
}

void module_remove_symbols(void *elf, size_t size) {
    const char *strtab;
    const struct elf_symbol *symtab;
    size_t strtab_size;
    size_t symtab_size;

    strtab = elf_get_section(elf, size, ".strtab", &strtab_size);
    if (strtab == NULL) {
        log(Log_Warning, "Failed to get .strtab when removing symbols");
        return;
    }

    symtab = elf_get_section(elf, size, ".symtab", &symtab_size);
    if (symtab == NULL) {
        log(Log_Warning, "Failed to get .symtab when removing symbols");
        return;
    }

    for (size_t i = 0; i < symtab_size / sizeof(struct elf_symbol); ++i) {
        if (ELF64_ST_BIND(symtab[i].st_info) == STB_GLOBAL) {

            assert_ok(symbol_remove(&strtab[symtab[i].st_name]));

            logf(Log_Info, "Removing symbol '%s'\n",
                 &strtab[symtab[i].st_name]);
        }
    }
}

enum status module_set_section_permissions(void *elf, size_t size) {
    enum status status;
    void *module_rw_start = elf_get_symbol_address(elf, size, "__module_rw_start");
    void *module_rw_end = elf_get_symbol_address(elf, size, "__module_rw_end");
    size_t rw_size = (module_rw_end - module_rw_start) / Page_Small;
    void *module_rx_start = elf_get_symbol_address(elf, size, "__module_rx_start");
    void *module_rx_end = elf_get_symbol_address(elf, size, "__module_rx_end");
    size_t rx_size = (module_rx_end - module_rx_start) / Page_Small;
    void *module_ro_start = elf_get_symbol_address(elf, size, "__module_ro_start");
    void *module_ro_end = elf_get_symbol_address(elf, size, "__module_ro_end");
    size_t ro_size = (module_ro_end - module_ro_start) / Page_Small;
    if (module_rw_start == NULL || module_rw_end == NULL ||
        module_rx_start == NULL || module_rx_end == NULL ||
        module_ro_start == NULL || module_ro_end == NULL) {
        log(Log_Error, "Couldn't get module delimiter\n");
        return Error_Permissions;
    }
    logf(Log_Info, "Regions:\nrx %p %p\nrw %p %p\nro %p %p\n", module_rx_start, module_rx_end, module_rw_start, module_rw_end, module_ro_start, module_ro_end);

    status = paging_update_range(module_rw_start, rw_size, Memory_Writable);
    if (status != Ok) {
        paging_update_range(module_rw_start, rw_size, Memory_No_Execute | Memory_Writable);
        return status;
    }

    status = paging_update_range(module_rx_start, rx_size, Memory_No_Attributes);
    if (status != Ok) {
        paging_update_range(module_rx_start, rx_size, Memory_No_Execute | Memory_Writable);
        paging_update_range(module_rw_start, rw_size, Memory_No_Execute | Memory_Writable);
        return status;
    }

    status = paging_update_range(module_ro_start, ro_size, Memory_No_Execute);
    if (status != Ok) {
        paging_update_range(module_ro_start, ro_size, Memory_No_Execute | Memory_Writable);
        paging_update_range(module_rx_start, rx_size, Memory_No_Execute | Memory_Writable);
        paging_update_range(module_rw_start, rw_size, Memory_No_Execute | Memory_Writable);
        return status;
    }

    return Ok;
}

enum status module_insert_symbols(void *elf, size_t size) {
    enum status status = Ok;
    struct elf64_header *header = elf;
    const char *strtab;
    const struct elf_symbol *symtab;
    const struct elf_section_header *section;
    size_t strtab_size;
    size_t symtab_size;
    void *location;

    strtab = elf_get_section(elf, size, ".strtab", &strtab_size);
    if (strtab == NULL) {
        return Error_Invalid;
    }

    symtab = elf_get_section(elf, size, ".symtab", &symtab_size);
    if (symtab == NULL) {
        return Error_Invalid;
    }

    for (size_t i = 0; i < symtab_size / sizeof(struct elf_symbol); ++i) {
        if (ELF64_ST_BIND(symtab[i].st_info) == STB_GLOBAL && (ELF64_ST_TYPE(symtab[i].st_info) == STT_FUNC || ELF64_ST_TYPE(symtab[i].st_info) == STT_OBJECT)) {
            section = elf_get_section_index(header, size, symtab[i].st_index);
            if (section == NULL) {
                status = Error_Invalid;
                goto err;
            }
            location = elf + section->sh_offset + symtab[i].st_value;

            status = symbol_put(&strtab[symtab[i].st_name], location);
            if (status != Ok) {
                goto err;
            }

            logf(Log_Info, "Symbol '%s': %p\n", &strtab[symtab[i].st_name], location);
        }
    }

    return status;
err:
    module_remove_symbols(elf, size);

    return status;
}


enum status module_load(void *module_start, size_t module_size) {
    enum status status;

    if (!elf_verify(module_start, module_size)) {
        return Error_Invalid;
    }

    const char *module_name = elf_get_shared_object_name(module_start,
                                                   module_size);
    logf(Log_Info, "Loading module %s\n", module_name);

    status = module_verify(module_start, module_size);
    if (status != Ok) {
        log(Log_Info, "Failed to verify module signature");
        return status;
    }

    status = elf_relocate(module_start, module_size);
    if (status != Ok) {
        log(Log_Info, "Failed to relocate ELF");
        return status;
    }

    status = module_insert_symbols(module_start, module_size);
    if (status != Ok) {
        log(Log_Info, "Failed to insert symbols");
        return status;
    }

    struct module *new_module = kmalloc(sizeof(struct module));
    if (new_module == NULL) {
        log(Log_Info, "Failed to allocate module structure");
        module_remove_symbols(module_start, module_size);
        return Error_No_Memory;
    }
    new_module->virtual_start = module_start;
    new_module->size = module_size;
    new_module->next = Module_List;
    new_module->name = module_name;

    Module_List = new_module;

    return Ok;
}


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
        size_t module_allocation_size = align_as(round_next(module_size, Page_Small), Page_Small);
        void *module_start = slab_alloc_request_physical(
                    modules[i].mod_start,
                    module_allocation_size,
                    Memory_Writable);
        if (module_start == NULL) {
            modules_status = Error_Invalid;
            continue;
        }

        enum status status = module_load(module_start, module_size);
        if (status != Ok) {
            log(Log_Error, "Error loading module");
            slab_free(Page_Small, module_start);
            modules_status = status;
            continue;
        }

        status = module_set_section_permissions(module_start, module_allocation_size);
        if (status != Ok) {
            log(Log_Error, "Error setting module section permissions");
            modules_status = status;
            slab_free(Page_Small, module_start);
            continue;
        }

        status = elf_run_init(module_start, module_size);
        if (status != Ok) {
            log(Log_Error, "Error running module init");
            modules_status = status;
            status = elf_run_fini(module_start, module_size);
            if (status != Ok) {
                log(Log_Error, "Error running module fini after init failed");
            }
            slab_free(Page_Small, module_start);
            continue;
        }

    }
    slab_free(Page_Small, modules);

    if (modules_status != Ok) {
        symbol_fini();
    }

    return modules_status;
}


void modules_info(struct multiboot_info *mi) {
    struct multiboot_mod_list *modules = (struct multiboot_mod_list *)(uintptr_t)mi->mods_addr;
    logf(Log_Debug, "%p\n", modules);
    for (size_t i = 0; i < mi->mods_count; ++i) {
        logf(Log_Debug, "start %x\n", modules[i].mod_start);
        logf(Log_Debug, "end %x\n", modules[i].mod_end);
        logf(Log_Debug, "file %s\n", (char *)(uintptr_t)modules[i].cmdline);
    }
}
