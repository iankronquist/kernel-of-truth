#include <arch/x64/paging.h>
#include <external/multiboot.h>
#include <truth/panic.h>
#include <truth/physical_allocator.h>
#include <truth/random.h>
#include <truth/region_vector.h>
#include <truth/slab.h>
#include <truth/types.h>

struct region {
    void *address;
    size_t size;
};

struct region_vector {
    size_t regions_used;
    struct region_vector *next;
    struct region regions[];
};

#define Region_Vector_Max_Random_Addresses 5

#define regions_count ( \
        (Page_Small - sizeof(struct region_vector)) / \
        sizeof(struct region) \
        )

void debug_region_vector(struct region_vector *cur) {
    do {
        logf(Log_Debug, "Region used %ld regions count %ld\n", cur->regions_used, regions_count);
        for (size_t i = 0; i < cur->regions_used && i < regions_count; ++i) {
            logf(Log_Debug, "Region starts at %p and has size %zu\n",
                 cur->regions[i].address, cur->regions[i].size);
        }
        cur = cur->next;
    } while (cur != NULL);
}

void region_vector_init(struct region_vector *vect) {
    vect->next = NULL;
    vect->regions_used = 0;
}

void region_vector_fini(struct region_vector *vect) {
    struct region_vector *prev = vect;
    while (vect != NULL) {
        vect = prev->next;
        slab_free(Page_Small, prev);
    }
}

struct region_vector *region_vector_new(void *addr, size_t size) {
    struct region_vector *vect = slab_alloc(Page_Small, Memory_Writable);
    if (vect == NULL) {
        return NULL;
    }

    region_vector_init(vect);
    region_free(vect, addr, size);
    return vect;
}

enum status checked region_alloc_random(struct region_vector *vect, size_t size, void **out) {
    assert(vect != NULL);
    assert(out != NULL);
    for (size_t addresses_tried = 0; addresses_tried < Region_Vector_Max_Random_Addresses; ++addresses_tried) {
        void *addr = random_address(true, Page_Small);
        struct region_vector *v = vect;
        do {
            for (size_t i = 0; i < v->regions_used && i < regions_count; ++i) {
                if (v->regions[i].address >= addr && v->regions[i].size > size) {
                    void *address = v->regions[i].address;
                    size_t new_size = v->regions[i].size - size;
                    if (new_size == 0) {
                        v->regions_used--;
                        if (i != v->regions_used) {
                            v->regions[i] = v->regions[v->regions_used];
                        }
                    } else {
                        v->regions[i].size = new_size;
                        v->regions[i].address += size;
                    }
                    *out = address;
                    return Ok;
                }
            }
            v = v->next;
        } while (v != NULL);

    }
    return Error_No_Memory;
}

enum status checked region_alloc(struct region_vector *vect, size_t size, void **out) {
    assert(vect != NULL);
    assert(out != NULL);
    do {
        for (size_t i = 0; i < vect->regions_used && i < regions_count; ++i) {
            if (vect->regions[i].size > size) {
                void *address = vect->regions[i].address;
                size_t new_size = vect->regions[i].size - size;
                if (new_size == 0) {
                    vect->regions_used--;
                    if (i != vect->regions_used) {
                        vect->regions[i] = vect->regions[vect->regions_used];
                    }
                } else {
                    vect->regions[i].size = new_size;
                    vect->regions[i].address += size;
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
                                           Memory_Writable);
    if (new == NULL) {
        return NULL;
    }
    new->regions_used = 0;
    new->next = NULL;
    return new;
}

size_t region_find_size_and_free(struct region_vector *vect,
                                 void *address) {
    struct region_vector *prev = NULL;
    struct region_vector *cur = vect;
    do {
        for (size_t i = 0; i < cur->regions_used && i < regions_count; ++i) {
            if (cur->regions[i].address== address) {
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

void region_free(struct region_vector *vect, void *address,
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
    logf(Log_Debug,"Region vector extended %p\n", new);
    assert(new != NULL);
    prev->next = new;
    new->regions_used = 1;
    new->regions[0].size = size;
    new->regions[0].address = address;
}
