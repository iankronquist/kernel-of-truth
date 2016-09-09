#include <arch/x86/paging.h>

#include <contrib/multiboot.h>

#include <truth/kassert.h>
#include <truth/klog.h>
#include <truth/kmem.h>
#include <truth/pmem.h>
#include <truth/vmem.h>

#include <truth/private/memlayout.h>

extern int bootstrap_heap;

void init_memory(phys_addr_t boot_tables) {
    status_t stat;
    kheap_install(&bootstrap_heap, PAGE_SIZE);
    // Identity map multiboot tables.
    // Mapping them anywhere else is more trouble than it's worth.
    void *mb_tables_page = (void*)PAGE_ALIGN(boot_tables);
    stat = map_page(get_cur_page_table(), mb_tables_page,
            PAGE_ALIGN(boot_tables), perm_none | perm_write);
    kassert(stat);
    struct multiboot_info *mb_tables = (void*)boot_tables;
    stat = init_phys_allocator(mb_tables);
    kassert(stat == Ok);
    stat = unmap_page(get_cur_page_table(), mb_tables_page, true);
    kassert(stat == Ok);
}
