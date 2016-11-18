#include <arch/x64/paging.h>
#include <external/multiboot.h>
#include <truth/types.h>
#include <truth/panic.h>
#include <truth/physical_allocator.h>
#include <truth/region_vector.h>
#include <truth/slab.h>

struct region {
    int tag;
    union region_address address;
    size_t size;
};

struct region_vector {
    size_t regions_used;
    //struct lock lock;
    struct region_vector *next;
    struct region regions[];
};

#define Regions_Count ( \
        (Page_Small - sizeof(struct region_vector)) / \
        sizeof(struct region) \
        )

static enum order region_compare_by_size(struct region *left,
                                         struct region *right) {
    if (left->size < right->size) {
        return Order_Less;
    } else if (left->size > right->size) {
        return Order_Greater;
    } else {
        return Order_Equal;
    }
}

static enum order region_compare_by_address(struct region *left,
                                            struct region *right) {
    if (left->address.virtual < right->address.virtual) {
        return Order_Less;
    } else if (left->address.virtual > right->address.virtual) {
        return Order_Greater;
    } else {
        return Order_Equal;
    }
}

static inline union region_address virt_as_region_address(void *v) {
    union region_address address;
    address.virtual = v;
    return address;
}

static inline union region_address phys_as_region_address(phys_addr p) {
    union region_address address;
    address.physical = p;
    return address;
}


void debug_region_vector(struct region_vector *cur) {
    logf("region vector %p [", cur);
    do {
        for (size_t i = 0; i < cur->regions_used && i < Regions_Count; ++i) {
            logf("{ address: %p, size: %lx },",
                 cur->regions[i].address.virtual, cur->regions[i].size);
        }
        cur = cur->next;
    } while (cur != NULL);
    logf("]\n");
}

void init_region_vector(struct region_vector *vect) {
    vect->next = NULL;
    vect->regions_used = 0;
}

static bool is_sorted_by_address(struct region_vector *v) {
    do {
        for (size_t i = 1; i < v->regions_used; ++i) {
            if (v->regions[i].address.virtual >
                v->regions[i-1].address.virtual) {
                return false;
            }
        }
        v = v->next;
    } while(v != NULL);
    return true;
}

static bool is_sorted_by_size(struct region_vector *v) {
    do {
        for (size_t i = 1; i < v->regions_used; ++i) {
            if (v->regions[i].size > v->regions[i-1].size) {
                return false;
            }
        }
        v = v->next;
    } while(v != NULL);
    return true;
}

static inline struct region *last_region(struct region_vector *vect) {
    assert(vect->regions_used > 0 && vect->regions_used <= Regions_Count);
    return &vect->regions[vect->regions_used-1];
}

static inline void region_swap(struct region *l, struct region *r) {
    if (l == r) {
        return;
    }
    struct region tmp = *l;
    *l = *r;
    *r = tmp;
}

void insertionsort(struct region_vector *vect, enum order (*comp)(struct region *left, struct region *right)) {
    for (size_t i = 0; i < vect->regions_used; ++i) {
        for (size_t j = i; j > 0; --j) {
            if (comp(&vect->regions[j - 1], &vect->regions[j]) == Order_Less) {
                region_swap(&vect->regions[j - 1], &vect->regions[j]);
            }
        }
    }
}

static void region_vector_sort(struct region_vector *vect, enum order (*comp)(struct region *left, struct region *right)) {
    if (vect->regions_used > 1) {
        insertionsort(vect, comp);
    }
}

static struct region *closest_larger_region_by_size(struct region_vector *vect,
                                                    size_t size,
                                                    struct region_vector **containing) {
    while(vect != NULL && last_region(vect)->size < size) {
        vect = vect->next;
    }
    if (vect == NULL) {
        return NULL;
    }

    struct region *closest_region = last_region(vect);
    int closest = INT_MAX;
    int left = 0;
    int right = vect->regions_used;
    int index = vect->regions_used / 2;

    while (index > 0) {
        int difference = vect->regions[index].size - size;

        if (difference < 0) {
            right = index;
        } else if (difference > 0) {
            if (closest > difference) {
                *containing = vect;
                closest_region = &vect->regions[index];
                closest = difference;
            }
            left = index;
        } else {
            *containing = vect;
            return &vect->regions[index];
        }
        index = (right - left) / 2;
    }
    *containing = vect;
    return closest_region;
}

static struct region *find_region_by_address(struct region_vector *vect, union region_address address, struct region_vector **containing) {
    while (vect != NULL &&
           last_region(vect)->address.virtual >
               address.virtual) {
        vect = vect->next;
    }
    if (vect == NULL) {
        return NULL;
    }
    int left = 0;
    int right = vect->regions_used;
    int index = (vect->regions_used - 1) / 2;
    while (index > 0) {
        if (vect->regions[index].address.virtual > address.virtual) {
            left = index;
        } else if (vect->regions[index].address.virtual < address.virtual) {
            right = index;
        } else {
            *containing = vect;
            return &vect->regions[index];
        }
        index = (right - left) / 2;
    }
    return NULL;
}


static enum status checked region_put(struct region_vector *vect,
                               union region_address address,
                               size_t size, int tag,
                               enum order (*comp)(struct region *left,
                                                  struct region *right)) {
    if (vect == NULL) {
        return Error_Invalid;
    }
    while (true) {
        // If there is room in the vector, place it at the end and sort it.
        if (vect->regions_used < Regions_Count) {
            vect->regions[vect->regions_used].size = size;
            vect->regions[vect->regions_used].address = address;
            vect->regions[vect->regions_used].tag = tag;
            vect->regions_used++;
            region_vector_sort(vect, comp);
            return Ok;
        } else if (vect->regions[Regions_Count-1].size < size) {
            // FIXME
            if (vect->next == NULL) {
                vect->next = slab_alloc(Page_Small, Memory_Writable, 'regv');
                if (vect->next == NULL) {
                    return Error_No_Memory;
                }
            }
            struct region tmp;
            tmp = vect->regions[Regions_Count-1];
            vect->regions[Regions_Count-1].size = size;
            vect->regions[Regions_Count-1].address = address;
            vect->regions[Regions_Count-1].tag = tag;
            size = tmp.size;
            address = tmp.address;
            tag = tmp.tag;
            region_vector_sort(vect, comp);
        } else if (vect->next == NULL) {
            vect->next = slab_alloc(Page_Small, Memory_Writable, 'regv');
            if (vect->next == NULL) {
                return Error_No_Memory;
            }
        }
        vect = vect->next;
    }
    return Ok;
}

static void region_remove(struct region_vector *vect, struct region *region,
                          enum order (*comp)(struct region *left,
                                             struct region *right)) {
    assert(vect->regions_used != 0);
    if (vect->regions_used != Regions_Count) {
        *region = *last_region(vect);
        vect->regions_used--;
        region_vector_sort(vect, comp);
    } else if (vect->next != NULL && vect->next->regions_used > 0) {
        *region = vect->next->regions[0];
        region_vector_sort(vect, comp);
        region_remove(vect->next, &vect->next->regions[0], comp);
        if (vect->next->regions_used == 0) {
            slab_free(vect->next, 'regv');
            vect->next = NULL;
        }
    } else {
        vect->regions_used--;
    }
}

enum status checked region_get_by_size(struct region_vector *vect, size_t size,
                                       union region_address *address) {
    struct region_vector *containing;
    size_t suffix_size;
    struct region *closest = closest_larger_region_by_size(vect, size,
                                                           &containing);
    if (closest == NULL) {
        log("a");
        return Error_No_Memory;
    }
    suffix_size = size - closest->size;
    *address = closest->address;
    region_remove(containing, closest, region_compare_by_size);
    if (suffix_size != 0) {
        assert_ok(region_put(vect,
                             virt_as_region_address(address->virtual +
                                                    suffix_size),
                             suffix_size, 'free', region_compare_by_size));
    }
    assert(is_sorted_by_size(vect));
    return Ok;
}


size_t region_get_by_address(struct region_vector *vect,
                             union region_address address, int tag) {
    struct region_vector *containing;
    struct region *closest = find_region_by_address(vect, address, &containing);
    if (closest == NULL) {
        return 0;
    }
    assert(closest->tag == tag);
    size_t size = closest->size;
    region_remove(containing, closest, region_compare_by_address);
    assert(is_sorted_by_address(vect));
    return size;
}

enum status checked region_put_by_size(struct region_vector *vect,
                                       union region_address address,
                                       size_t size, int tag) {
    assert(is_sorted_by_size(vect));
    enum status status =  region_put(vect, address, size, tag, region_compare_by_size);
    assert(is_sorted_by_size(vect));
    return status;
}

enum status checked region_put_by_address(struct region_vector *vect,
                                          union region_address address,
                                          size_t size, int tag) {
    assert(is_sorted_by_address(vect));
    enum status status = region_put(vect, address, size, tag, region_compare_by_address);
    assert(is_sorted_by_address(vect));
    return status;
}


