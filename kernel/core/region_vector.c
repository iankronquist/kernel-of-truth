#include <arch/x64/paging.h>
#include <external/multiboot.h>
#include <truth/types.h>
#include <truth/panic.h>
#include <truth/physical_allocator.h>
#include <truth/region_vector.h>
#include <truth/slab.h>

struct region {
    union address address;
    size_t size;
};

struct region_vector {
    size_t regions_used;
    struct region_vector *next;
    struct region regions[];
};

#define regions_count ( \
        (Page_Small - sizeof(struct region_vector)) / \
        sizeof(struct region) \
        )

void debug_region_vector(struct region_vector *cur) {
    do {
        for (size_t i = 0; i < cur->regions_used && i < regions_count; ++i) {
            logf(Log_Debug, "Region starts at %p and has size %zu\n",
                 cur->regions[i].address.virtual, cur->regions[i].size);
        }
        cur = cur->next;
    } while (cur != NULL);
}

void region_vector_init(struct region_vector *vect) {
    vect->next = NULL;
    vect->regions_used = 0;
}

enum status checked region_alloc(struct region_vector *vect, size_t size,
                                 union address *out) {
    do {
        for (size_t i = 0; i < vect->regions_used && i < regions_count; ++i) {
            if (vect->regions[i].size > size) {
                union address address = vect->regions[i].address;
                size_t new_size = vect->regions[i].size - size;
                if (new_size == 0) {
                    vect->regions_used--;
                    if (i != vect->regions_used) {
                        vect->regions[i] = vect->regions[vect->regions_used];
                    }
                } else {
                    vect->regions[i].size = new_size;
                    vect->regions[i].address.physical += size;
                }
                *out = address;
                return Ok;
            }
        }
        vect = vect->next;
    } while (vect != NULL);
    return Error_No_Memory;
}

static struct region_vector *extend_vector(void) {
    struct region_vector *new = slab_alloc_locked(Page_Small,
                                           Memory_No_Attributes);
    if (new == NULL) {
        return NULL;
    }
    new->regions_used = 0;
    new->next = NULL;
    return new;
}

size_t region_find_size_and_free(struct region_vector *vect,
                                 union address address) {
    struct region_vector *prev = NULL;
    struct region_vector *cur = vect;
    do {
        for (size_t i = 0; i < cur->regions_used && i < regions_count; ++i) {
            if (cur->regions[i].address.virtual == address.virtual) {
                cur->regions_used--;
                size_t size = cur->regions[i].size;
                if (cur->regions_used != 0) {
                    cur->regions[i] = cur->regions[cur->regions_used];
                } else if (prev != NULL) {
                    prev->next = cur->next;
                    slab_free_locked(Page_Small, cur);
                }
                return size;
            }
        }
        prev = cur;
        cur = cur->next;
    } while (cur != NULL);
    return 0;
}

void region_free(struct region_vector *vect, union address address,
                 size_t size) {
    struct region_vector *prev = vect;
    struct region_vector *cur = vect;
    do {
        if (cur->regions_used != regions_count) {
            cur->regions[cur->regions_used].address = address;
            cur->regions[cur->regions_used].size = size;
            cur->regions_used++;
            return;
        }
        prev = cur;
        cur = cur->next;
    } while (cur != NULL);
    struct region_vector *new = extend_vector();
    assert(new != NULL);
    prev->next = new;
    new->regions_used = 1;
    new->regions[0].size = size;
    new->regions[0].address = address;
}
