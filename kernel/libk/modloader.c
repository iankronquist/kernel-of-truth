#include <libk/klog.h>
#include <libk/kmem.h>
#include <libk/modloader.h>
#include <libk/mod.h>

extern int kernel_symbols_start;
extern int kernel_symbols_end;

struct kernel_symbol {
    uint32_t loc;
    char *name;
};

static struct multiboot_mod_list *boot_modules;
static size_t num_boot_modules;

/*
static struct module *loaded_modules;
static size_t num_loaded_modules;
*/

int persist_multiboot_module_data(struct multiboot_info *mb) {
    size_t mods_alloc = mb->mods_count * sizeof(struct multiboot_mod_list);
    boot_modules = kmalloc(mods_alloc);
    num_boot_modules = mb->mods_count;
    memcpy(boot_modules, (void*)mb->mods_addr, mods_alloc);
    return 0;
}

int load_boot_modules(void) {
    kassert(0);
    /*
    num_loaded_modules = num_boot_modules;
    klogf("Kernel symbols range from %p to %p\n", &kernel_symbols_start,
          &kernel_symbols_end);
    for (struct kernel_symbol *ks = (void*)&kernel_symbols_start;
            ks < (struct kernel_symbol*)&kernel_symbols_end; ++ks) {
        klogf("%s: %p\n", ks->name, &ks->loc);
    }

    // The memory is reserved by the physical allocator, but not paged.
    // Page it, load the elf binary, unpage it, and free it.
    for (size_t i  = 0; i < num_boot_modules; ++i) {
        //for (
        //void *multiboot_blob = find_free_addr(uint32_t *page_dir, boot_modules[i]->mod_start, 
        uint16_t permissions);
        free_range(boot_modules[i]->mod_start, boot_modules[i]->mod_end);
    }
    // The list of boot modules is no longer needed once all of the modules are
    // loaded.
    */
    kfree(boot_modules);
    return 0;
}
