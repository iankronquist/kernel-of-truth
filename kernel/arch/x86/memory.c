#include <arch/x86/paging.h>

#include <contrib/multiboot.h>

#include <truth/klog.h>
#include <truth/kmem.h>
#include <truth/physical_allocator.h>
#include <truth/vmem.h>

#include <truth/private/memlayout.h>

static page_frame_t kernel_page_table_install(struct multiboot_info *mb);

void init_memory(void *boot_tables) {
    struct multiboot_info *mb = boot_tables;
    kheap_install((struct kheap_metadata*)KHEAP_PHYS_ROOT, PAGE_SIZE);
    physical_allocator_init(mb->mem_upper + mb->mem_lower);
    page_frame_t unused(pft) = kernel_page_table_install(mb);

}

static page_frame_t kernel_page_table_install(struct multiboot_info *mb) {
    // Certain very important things already exist in physical memory. They
    // need to be marked as present so that the allocator doesn't grab them by
    // accident.
    // After they are marked as present they can safely be mapped with
    // map_page.

    // Iterate over the multiboot memory map table and mark certain addresses
    // as unusable.
    klogf("Marking unusable!\n");
    multiboot_memory_map_t *mm_last = (multiboot_memory_map_t*)(mb->mmap_addr +
        mb->mmap_length);
    for (multiboot_memory_map_t *mm = (multiboot_memory_map_t*)mb->mmap_addr;
         mm < mm_last;
         mm = (multiboot_memory_map_t*)((uintptr_t)mm +
                                        mm->size + sizeof(mm->size))) {
        // If the memory is not available
        if (mm->type != MULTIBOOT_MEMORY_AVAILABLE) {
            klogf("Unusable physical address %p of type %p and length %p\n",
                    mm->addr, mm->type, mm->len);
            for (uint64_t page = PAGE_ALIGN(mm->addr);
                 page < NEXT_PAGE(mm->addr+mm->len); page += PAGE_SIZE) {
                use_frame(page);
            }
        }
    }
    klogf("mm_addr table %p\n", mb->mmap_addr);
    klogf("apm table %p\n", mb->apm_table);
    klogf("fb table %p\n", mb->framebuffer_addr);
    klogf("vbe int off %p\n", mb->vbe_interface_off);

    // Catch NULL pointer dereferences
    use_frame(0);

    // Mark all the pages the kernel sits on as used
    use_range(KERNEL_START, KERNEL_END);

    // Mark the kernel heap as in use
    use_frame(KHEAP_PHYS_ROOT);
    // Mark video memory as in use
    use_range(VGA_BEGIN, VGA_END);

    // Mark the paging directory as in use
    use_frame(PAGE_DIRECTORY);
    page_frame_t page_dir = bootstrap_kernel_page_table();

    enable_paging(page_dir);
    return page_dir;
}
