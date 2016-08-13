#include <arch/x86/paging.h>
#include <arch/x86/process.h>

#include <truth/kassert.h>
#include <truth/klog.h>
#include <truth/pmem.h>
#include <truth/types.h>

#include <truth/private/memlayout.h>

struct region_head {
    struct region *list;
};

struct region {
    uint64_t size;
    void *addr;
    struct region *next;
};

static struct region *init_region(void *address, uint64_t size,
        struct region *next);

// TODO: Make this an rb tree insertion. This is O(N) linked list insertion.
// TODO: Implement region merging.
status_t checked insert_region(void *addr, uint64_t size,
        struct region_head *head) {
    kassert(head != NULL);

    // Walk the list until we either reach an element bigger than the current
    // size or we reach the end.
    struct region *prev = NULL;
    struct region *cur = head->list;
    while (cur != NULL && size < cur->size) {
        prev = cur;
        cur = cur->next;
    }

    // Allocate new region.
    struct region *new = init_region(addr, size, cur);
    if (new == NULL) {
        return Err;
    }

    // If we should insert it at the head of the list.
    if (prev == NULL) {
        head->list = new;
    } else { // Otherwise insert the region into the middle of the list.
        prev->next = new;
    }

    return Ok;
}

// TODO: Make this an rb tree walk. This is O(N) linked list traversal.
void *find_region(size_t size, struct region_head *head) {
    struct region *prev = NULL;
    struct region *cur = head->list;
    while (cur != NULL && size > cur->size) {
        prev = cur;
        cur = cur->next;
    }
    // If there isn't enough space.
    if (cur == NULL) {
        return NULL;
    }
    // If we have a previous element, remove the region we found from the list.
    if (prev != NULL) {
        prev->next = cur->next;
    } else {
        // Otherwise the list is empty.
        head->list = NULL;
    }
    klogf("closest %p %p\n", cur->addr, cur->size);
    // If the region is too big, split it and insert the smaller end into the
    // list.
    if (cur->size > size) {
        void *split_addr = cur->addr + (size * PAGE_SIZE);
        size_t split_size = cur->size - size;
        cur->size = size;
        status_t stat = insert_region(split_addr, split_size, head);
        if (stat != Ok) {
            return NULL;
        }
    }
    void *addr = cur->addr;
    kfree(cur);
    return addr;
}

static struct region *init_region(void *address, uint64_t size,
        struct region *next) {
    struct region *r = kmalloc(sizeof(struct region));
    r->size = size;
    r->addr = address;
    r->next = next;
    return r;
}

struct region_head *init_region_list(void) {
    struct region_head *new = kmalloc(sizeof(struct region));
    new->list = NULL;
    return new;
}

void destroy_free_list(struct region_head *head) {
    struct region *cur = head->list;
    while (cur != NULL) {
        struct region *next = cur->next;
        kfree(cur);
        cur = next;
    }
    kfree(cur);
    kfree(head);
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
