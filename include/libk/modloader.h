#ifndef MODLOADER_H
#define MODLOADER_H
#include <contrib/multiboot.h>


// Copy multiboot tables onto the kernel heap which will be paged.
// When we switch to paging the multiboot tables will be destroyed.
// Must be called before paging is enabled, but after the heap is initialized.
int persist_multiboot_module_data(struct multiboot_info *mb);

// Loads kernel modules passed by multiboot.
// Allocates paged memory for modules. Must be called after paging is enabled.
// Also frees physical memory pages which the multiboot module blobs sit on.
int load_boot_modules(void);

#endif
