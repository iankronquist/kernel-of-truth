#include <arch/x86/paging.h>
#include <arch/x86/process.h>

#include <truth/kassert.h>
#include <truth/pmem.h>
#include <truth/types.h>

#include <truth/private/memlayout.h>

struct region {
    uint64_t size;
    void *addr;
    struct region *next;
};

// TODO: Make this an rb tree insertion. This is O(N) linked list insertion.
// TODO: Implement region merging.
status_t checked insert_region(void *addr, size_t size, struct region *vr) {
    struct region *prev = vr;
    kassert(prev != NULL);
    struct region *cur = vr;
    while (cur != NULL && size < cur->size) {
        prev = cur;
        cur = cur->next;
    }
    struct region *new = kmalloc(sizeof(struct region));
    if (new != 0) {
        new->size = size;
        new->addr = addr;
        new->next = cur;
        prev->next = new;
        return Ok;
    } else {
        return Err;
    }
}

// TODO: Make this an rb tree walk. This is O(N) linked list traversal.
void *find_region(size_t size, struct region *vr) {
    struct region *closest_prev = NULL;
    struct region *closest = NULL;
    struct region *prev = NULL;
    struct region *cur = vr;
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
        status_t stat = insert_region(split_addr, split_size, vr);
        if (stat != Ok) {
            return NULL;
        }
    }
    void *addr = closest->addr;
    kfree(closest);
    return addr;
}

struct region *init_region(void *address, size_t size, struct region *next) {
    struct region *r = kmalloc(sizeof(struct region));
    r->size = size;
    r->addr = address;
    r->next = next;
    return r;
}

void destroy_free_list(struct region *vr) {
    struct region *cur = vr;
    while (cur->next != NULL) {
        struct region *next = cur->next;
        kfree(cur);
        cur = next;
    }
    kfree(cur);
}

void map_region(void *vr, size_t pages,  uint16_t perms) {
    for (void *addr = vr; addr < vr + (pages * PAGE_SIZE);
            addr += PAGE_SIZE) {
        inner_map_page(CUR_PAGE_DIRECTORY_ADDR, alloc_frame(), addr, perms);
    }
}

void unmap_region(void *vr, size_t pages) {
    for (void *addr = vr; addr < vr + (pages * PAGE_SIZE);
            addr += PAGE_SIZE) {
        inner_unmap_page(CUR_PAGE_DIRECTORY_ADDR, addr, true);
    }
}
