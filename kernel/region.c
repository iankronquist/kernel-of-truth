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
void debug_region(struct region_head *head);

// TODO: Make this an rb tree insertion. This is O(N) linked list insertion.
// TODO: Implement region merging.
status_t checked insert_region(void *addr, uint64_t size,
        struct region_head *head) {
    kassert(PAGE_ALIGN(size) == size);
    kassert((void*)PAGE_ALIGN(addr) == addr);
    kassert(head != NULL);
    klogf("before insertion\n");

    // Walk the list until we either reach an element bigger than the current
    // size or we reach the end.
    struct region *prev = NULL;
    struct region *cur = head->list;
    while (cur != NULL && size > cur->size) {
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
    klogf("Insert region %p of size %p\n", addr, size);

    return Ok;
}

// TODO: Make this an rb tree walk. This is O(N) linked list traversal.
void *find_region(size_t size, struct region_head *head) {
    kassert(PAGE_ALIGN(size) == size);
    kassert(head != NULL);
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
        //head->list = NULL;
        head->list = cur->next;
    }
    // If the region is too big, split it and insert the smaller end into the
    // list.
    if (cur->size > size) {
        void *split_addr = cur->addr + size;
        size_t split_size = cur->size - size;
        cur->size = size;
        status_t stat = insert_region(split_addr, split_size, head);
        if (stat != Ok) {
            kassert(0);
            return NULL;
        }
    }
    void *addr = cur->addr;
    kfree(cur);
    kassert((void*)PAGE_ALIGN(addr) == addr);
    return addr;
}

void debug_region(struct region_head *head) {
    struct region *cur = head->list;
    klogf("region %p: [", head);

    while (cur != NULL) {
        klogf("{ addr: %p, size: %p }, ", cur->addr, cur->size);
        cur = cur->next;
    }
    klogf("]\n");
}

status_t checked remove_region(void *addr, size_t size,
        struct region_head *head) {
    kassert(PAGE_ALIGN(size) == size);
    kassert((void*)PAGE_ALIGN(addr) == addr);
    status_t stat;
    struct region *prev = NULL;
    struct region *cur = head->list;
    while (cur != NULL) {
        // If the address is in the current region
        if (cur->addr <= addr && cur->addr + cur->size >= addr) {
            // Remove the current region from the list.
            if (prev != NULL) {
                prev->next = cur->next;
            } else {
                head->list = cur->next;
            }
            // If there is space before the desired address, put it into the
            // list.
            if (cur->addr < addr) {
                stat = insert_region(cur->addr, addr - cur->addr, head);
                if (stat != Ok) {
                    kfree(cur);
                    klog("error inserting prefix region");
                    return stat;
                }
            }

            void *cur_end = cur->addr + cur->size;
            void *new_end = addr + size;
            if (cur_end > new_end) {
                klog("suffix\n");
                // If the current region has more space at the end,
                // put that space back into the list.
                stat = insert_region(new_end, cur_end - new_end, head);
                if (stat != Ok) {
                    kfree(cur);
                    klog("error inserting suffix region");
                    return stat;
                }
            } else if (cur_end < new_end) {
                klog("more\n");
                // If the current region is too small, remove more.
                // This is guaranteed to terminate because we assume that
                // new_end always increases.
                stat = remove_region(new_end, new_end - cur_end, head);
                if (stat != Ok) {
                    kfree(cur);
                    klog("error removing suffix region");
                    return stat;
                }
            }
            kfree(cur);
            return Ok;
        }
        prev = cur;
        cur = cur->next;
    }
    klog("error falling out bottom");
    // If there isn't enough space or we couldn't find it, error.
    return Err;
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

void map_region(void *vr, size_t pages,  enum memory_permissions perms) {
    kassert((void*)PAGE_ALIGN(vr) == vr);
    for (void *addr = vr; addr < vr + pages;
            addr += PAGE_SIZE) {
        // FIXME check this.
        status_t unused(stat);
        stat = map_page(get_cur_page_table(), addr, alloc_frame(), perms);
    }
}

void map_region_page(void *vr, page_frame_t page, size_t pages,
        uint16_t perms) {
    kassert((void*)PAGE_ALIGN(vr) == vr);
    for (void *addr = vr; addr < vr + pages;
            addr += PAGE_SIZE, page += PAGE_SIZE) {
        // FIXME check this.
        status_t unused(stat);
        stat = map_page(get_cur_page_table(), addr, page, perms);

    }
}

void unmap_region(void *vr, size_t pages) {
    kassert((void*)PAGE_ALIGN(vr) == vr);
    for (void *addr = vr; addr < vr + pages;
            addr += PAGE_SIZE) {
        // FIXME check this.
        status_t unused(stat);
        stat = unmap_page(get_cur_page_table(), addr, true);
    }
}
