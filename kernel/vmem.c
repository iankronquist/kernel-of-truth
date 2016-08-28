#include <arch/x86/process.h>

#include <truth/kassert.h>
#include <truth/types.h>
#include <truth/pmem.h>
#include <truth/vmem.h>

#include <truth/private/memlayout.h>
#include <truth/private/region.h>

static inline virt_region_t *get_cur_free_regions(void) {
    return get_current_proc()->memory.free_virt;
}

virt_region_t *init_free_list(void) {
    virt_region_t *new = init_region_list();
    // Don't give out the NULL page.
    status_t stat = insert_region(NULL + PAGE_SIZE, KERNEL_START - PAGE_SIZE, new);
    if (stat != Ok) {
        destroy_free_list(new);
        return NULL;
    }
    return new;
}

void *get_virt_region(size_t pages, enum region_perms perms) {
    void *addr = find_region(pages, get_cur_free_regions());
    map_region(addr, alloc_frame(), (uint16_t)perms);
    return addr;
}

void put_virt_region(void *region_start, size_t pages) {
    unmap_region(region_start, pages);
    status_t stat = insert_region(region_start, pages, get_cur_free_regions());
    kassert(stat != Err);
}
