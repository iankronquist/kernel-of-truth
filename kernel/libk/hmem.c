#include <truth/hmem.h>
#include <truth/kassert.h>
#include <truth/lock.h>
#include <truth/pmem.h>
#include <truth/types.h>

#include <truth/private/memlayout.h>
#include <truth/private/region.h>

spinlock_t higher_half_lock = SPINLOCK_INIT;
phys_region_t *higher_half_root = NULL;

status_t checked init_higher_half(page_frame_t highest_address) {
    status_t stat = Err;
    acquire_spinlock(&higher_half_lock);
    higher_half_root = init_region_list();
    if (higher_half_root != NULL) {
        stat = insert_region((void*)KHEAP_PHYS_END,
                highest_address - (uintptr_t)KHEAP_PHYS_END, higher_half_root);
    }
    release_spinlock(&higher_half_lock);
    return stat;
}

page_frame_t get_kernel_region(size_t pages, enum region_perms perms) {
    acquire_spinlock(&higher_half_lock);
    void *addr = find_region(pages, higher_half_root);
    release_spinlock(&higher_half_lock);
    map_region(addr, alloc_frame(), (uint16_t)perms);
    return (page_frame_t)addr;
}

void put_kernel_region(page_frame_t region, size_t pages) {
    unmap_region((void*)region, pages);
    acquire_spinlock(&higher_half_lock);
    status_t stat = insert_region((void*)region, pages, higher_half_root);
    kassert(stat == Ok);
    release_spinlock(&higher_half_lock);
}
