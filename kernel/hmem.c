#include <truth/hmem.h>
#include <truth/kassert.h>
#include <truth/lock.h>
#include <truth/pmem.h>
#include <truth/types.h>

#include <truth/private/memlayout.h>
#include <truth/private/region.h>

spinlock_t higher_half_lock = SPINLOCK_INIT;
phys_region_t *higher_half_root = NULL;

status_t checked init_higher_half(void) {
    uintptr_t highest_address = ~0;
    status_t stat = Err;
    acquire_spinlock(&higher_half_lock);
    higher_half_root = init_region_list();
    if (higher_half_root != NULL) {
        stat = insert_region((void*)KHEAP_PHYS_END,
                ROUND_NEXT_PAGE(highest_address - (uintptr_t)KHEAP_PHYS_END),
                higher_half_root);
    }
    release_spinlock(&higher_half_lock);
    return stat;
}

void *get_kernel_region(size_t pages, enum region_perms perms) {
    acquire_spinlock(&higher_half_lock);
    void *addr = find_region(pages, higher_half_root);
    release_spinlock(&higher_half_lock);
    map_region(addr, pages, (uint16_t)perms);
    return addr;
}

void put_kernel_region(void *region, size_t pages) {
    unmap_region((void*)region, pages);
    acquire_spinlock(&higher_half_lock);
    status_t stat = insert_region(region, pages, higher_half_root);
    kassert(stat == Ok);
    release_spinlock(&higher_half_lock);
}

void *get_kernel_region_page(size_t pages, page_frame_t first_page,
        enum region_perms perms) {
    acquire_spinlock(&higher_half_lock);
    void *addr = find_region(pages, higher_half_root);
    release_spinlock(&higher_half_lock);
    map_region(addr, first_page, (uint16_t)perms);
    return addr;
}
