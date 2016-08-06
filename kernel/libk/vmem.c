#include <arch/x86/paging.h>
#include <arch/x86/process.h>

#include <truth/kassert.h>
#include <truth/kmem.h>
#include <truth/types.h>
#include <truth/lock.h>
#include <truth/physical_allocator.h>
#include <truth/vmem.h>

#include <truth/private/memlayout.h>

struct virt_region {
    size_t size;
    void *addr;
    struct virt_region *next;
};

static inline struct virt_region *get_cur_free_regions(void) {
    return get_current_proc()->memory.free_virt;
}

static void map_region(void *vr, size_t pages,  uint16_t perms) {
    for (void *addr = vr; addr < vr + (pages * PAGE_SIZE);
            addr += PAGE_SIZE) {
        inner_map_page(CUR_PAGE_DIRECTORY_ADDR, alloc_frame(), addr, perms);
    }
}

static void unmap_region(void *vr, size_t pages) {
    for (void *addr = vr; addr < vr + (pages * PAGE_SIZE);
            addr += PAGE_SIZE) {
        inner_unmap_page(CUR_PAGE_DIRECTORY_ADDR, addr, true);
    }
}

// TODO: Make this an rb tree insertion. This is O(N) linked list insertion.
// TODO: Implement region merging.
static void insert_region(void *addr, size_t size, struct virt_region *vr) {
    struct virt_region *prev = vr;
    kassert(prev != NULL);
    struct virt_region *cur = vr;
    while (cur != NULL && size < cur->size) {
        prev = cur;
        cur = cur->next;
    }
    struct virt_region *new = kmalloc(sizeof(struct virt_region));
    new->size = size;
    new->addr = addr;
    new->next = cur;
    prev->next = new;
}

// TODO: Make this an rb tree walk. This is O(N) linked list traversal.
static void *find_region(size_t size, struct virt_region *vr) {
    struct virt_region *closest_prev = NULL;
    struct virt_region *closest = NULL;
    struct virt_region *prev = NULL;
    struct virt_region *cur = vr;
    size_t closest_size = SIZE_MAX;
    while (cur->next != NULL && cur->size < cur->next->size) {
        if (cur->size >= size && closest_size > cur->size - size) {
            closest_size = cur->size;
            closest = cur;
            closest_prev = prev;
        }
        prev = cur;
        cur = cur->next;
    }
    // If there isn't enough space.
    if (closest == NULL) {
        return NULL;
    }
    // Remove the closest match from the list.
    if (closest_prev != NULL) {
        closest_prev->next = closest->next;
    }
    // If the region is too big, split it and insert the smaller end into the
    // list.
    if (closest_size > size) {
        void *split_addr = closest->addr + (size * PAGE_SIZE);
        size_t split_size = closest->size - size;
        closest->size = size;
        insert_region(split_addr, split_size, vr);
    }
    void *addr = closest->addr;
    kfree(closest);
    return addr;
}

struct virt_region *init_free_list(void) {
    struct virt_region *lower_half = kmalloc(sizeof(struct virt_region));
    // Don't give out the NULL page.
    lower_half->size = KERNEL_START - PAGE_SIZE;
    lower_half->addr = NULL + PAGE_SIZE;
    struct virt_region *higher_half = kmalloc(sizeof(struct virt_region));
    higher_half->size = ((void*)~0) - KHEAP_PHYS_END;
    higher_half->addr = KHEAP_PHYS_END;
    higher_half->next = NULL;
    lower_half->next = higher_half;
    return lower_half;
}

void destroy_free_list(struct virt_region *vr) {
    struct virt_region *cur = vr;
    while (cur->next != NULL) {
        struct virt_region *next = cur->next;
        kfree(cur);
        cur = next;
    }
    kfree(cur);
}

void *get_region(size_t pages, enum region_perms perms) {
    void *addr = find_region(pages, get_cur_free_regions());
    map_region(addr, alloc_frame(), (uint16_t)perms);
    return addr;
}

void put_region(void *region, size_t pages) {
    unmap_region(region, pages);
    insert_region(region, pages, get_cur_free_regions());
}
