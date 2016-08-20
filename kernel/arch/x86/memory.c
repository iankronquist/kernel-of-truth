#include <arch/x86/paging.h>

#include <contrib/multiboot.h>

#include <truth/kassert.h>
#include <truth/klog.h>
#include <truth/kmem.h>
#include <truth/pmem.h>
#include <truth/vmem.h>

#include <truth/private/memlayout.h>

void init_memory(void *boot_tables) {
    kheap_install((void*)KHEAP_PHYS_ROOT, PAGE_SIZE);
    status_t stat = init_phys_allocator(boot_tables, 0);
    kassert(stat == Ok);
    page_frame_t page_dir = bootstrap_kernel_page_table();
    enable_paging(page_dir);
}
