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
    struct region_vector *next;
    struct region regions[];
};

#define Regions_Count ( \
        (Page_Small - sizeof(struct region_vector)) / \
        sizeof(struct region) \
        )

void debug_region_vector(struct region_vector *cur) {
    do {
        for (size_t i = 0; i < cur->regions_used && i < Regions_Count; ++i) {
            logf("Region starts at %p and has size %x\n",
                 cur->regions[i].address.virtual, cur->regions[i].size);
        }
        cur = cur->next;
    } while (cur != NULL);
}

void init_region_vector(struct region_vector *vect) {
    vect->next = NULL;
    vect->regions_used = 0;
}

static bool is_sorted_by_address(struct region_vector *v) {
    do {
        for (size_t i = 1; i < v->regions_used; ++i) {
            if (v->regions[i].address.virtual < v->regions[i-1].address.virtual) {
                return false;
            }
        }
    } while(v != NULL);
    return true;
}

static bool is_sorted_by_size(struct region_vector *v) {
    do {
        for (size_t i = 1; i < v->regions_used; ++i) {
            if (v->regions[i].size < v->regions[i-1].size) {
                return false;
            }
        }
    } while(v != NULL);
    return true;
}

static struct region *closest_larger_region_by_size(struct region_vector *vect, size_t size, struct region_vector **containing) {
    assert(vect->regions_used > 0);
    while(vect != NULL && vect->regions[vect->regions_used-1].size < size) {
        vect = vect->next;
    }
   if (vect == NULL) {
        return NULL;
    }

    struct region *closest_region = NULL;
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
    return closest_region;
}

static struct region *find_region_by_address(struct region_vector *vect, union region_address address, struct region_vector **containing) {
    while (vect != NULL &&
           vect->regions[vect->regions_used-1].address.virtual >
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

static inline size_t right_child(size_t i) {
    return 2 * i + 1;
}

static inline size_t left_child(size_t i) {
    return 2 * i;
}


static void region_vector_sift_down_by_size(struct region_vector *vect,
                                            size_t begin, size_t end) {
    size_t root = begin;
    while (left_child(root) < end) {
        size_t swap = root;
        size_t child = left_child(root);

        if (child + 1 < end &&
            vect->regions[swap].size < vect->regions[child+1].size) {
            swap = child + 1;
        } else if (vect->regions[swap].size < vect->regions[child].size) {
            swap = child;
        }

        if (swap == root) {
            return;
        } else {
            struct region tmp = vect->regions[root];
            vect->regions[root] = vect->regions[swap];
            vect->regions[swap] = tmp;
        }
    }
}

static void region_vector_sift_down_by_address(struct region_vector *vect,
                                            size_t begin, size_t end) {
    size_t root = begin;
    while (left_child(root) < end) {
        size_t swap = root;
        size_t child = left_child(root);

        if (child + 1 < end &&
            vect->regions[swap].address.virtual < vect->regions[child+1].address.virtual) {
            swap = child + 1;
        } else if (vect->regions[swap].address.virtual < vect->regions[child].address.virtual) {
            swap = child;
        }

        if (swap == root) {
            return;
        } else {
            struct region tmp = vect->regions[root];
            vect->regions[root] = vect->regions[swap];
            vect->regions[swap] = tmp;
        }
    }
}


static void region_vector_heapify_by_size(struct region_vector *vect) {
    for (size_t start = (vect->regions_used - 1) / 2; start != 0; --start) {
        region_vector_sift_down_by_size(vect, start, vect->regions_used-1);
    }
    region_vector_sift_down_by_size(vect, 0, vect->regions_used-1);
}


static void region_vector_heapify_by_address(struct region_vector *vect) {
    for (size_t start = (vect->regions_used - 1) / 2; start != 0; --start) {
        region_vector_sift_down_by_address(vect, start, vect->regions_used-1);
    }
    region_vector_sift_down_by_address(vect, 0, vect->regions_used-1);
}


static void region_vector_sort_by_size(struct region_vector *vect) {
    do {
        if (vect->regions_used == 1) {
            continue;
        }
        region_vector_heapify_by_size(vect);
        size_t end = vect->regions_used;
        while (end > 0) {
            struct region tmp;
            tmp = vect->regions[end];
            vect->regions[end] = vect->regions[0];
            vect->regions[0] = tmp;
            end--;
            region_vector_sift_down_by_size(vect, 0, end);
        }
        assert(is_sorted_by_size(vect));
        if (vect->next != NULL) {
            assert(vect->regions[vect->regions_used-1].size > vect->next->regions[0].size);
        }

    } while(vect != NULL);
}

static void region_vector_sort_by_address(struct region_vector *vect) {
    do {
        if (vect->regions_used == 1) {
            continue;
        }
        region_vector_heapify_by_address(vect);
        size_t end = vect->regions_used;
        while (end > 0) {
            struct region tmp;
            tmp = vect->regions[end];
            vect->regions[end] = vect->regions[0];
            vect->regions[0] = tmp;
            end--;
            region_vector_sift_down_by_address(vect, 0, end);
        }
        assert(is_sorted_by_address(vect));
        if (vect->next != NULL) {
            assert(vect->regions[vect->regions_used-1].address.virtual > vect->next->regions[0].address.virtual);
        }

    } while(vect != NULL);
}


enum status checked region_put_by_size(struct region_vector *vect,
                                       union region_address address,
                                       size_t size, int tag) {
    if (vect == NULL) {
        return Error_Invalid;
    }
    while (true) {
        if (vect->regions_used < Regions_Count) {
            vect->regions[vect->regions_used].size = size;
            vect->regions[vect->regions_used].address = address;
            vect->regions[vect->regions_used].tag = tag;
            vect->regions_used++;
            region_vector_sort_by_size(vect);
            return Ok;
        } else if (vect->regions[Regions_Count-1].size < size) {
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
            region_vector_sort_by_size(vect);
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

static void region_remove_by_size(struct region_vector *vect,
                                  struct region *region) {
    assert(vect->regions_used != 0);
    if (vect->regions_used != Regions_Count) {
        *region = vect->regions[vect->regions_used-1];
        vect->regions_used--;
        region_vector_sort_by_size(vect);
    } else if (vect->next != NULL && vect->next->regions_used > 0) {
        *region = vect->next->regions[0];
        region_vector_sort_by_size(vect);
        region_remove_by_size(vect->next, &vect->next->regions[0]);
        if (vect->next->regions_used == 0) {
            slab_free(vect->next, 'regv');
            vect->next = NULL;
        }
    } else {
        vect->regions_used--;
    }
}

static void region_remove_by_address(struct region_vector *vect,
                                  struct region *region) {
    assert(vect->regions_used != 0);
    if (vect->regions_used != Regions_Count) {
        *region = vect->regions[vect->regions_used-1];
        vect->regions_used--;
        region_vector_sort_by_address(vect);
    } else if (vect->next != NULL && vect->next->regions_used > 0) {
        *region = vect->next->regions[0];
        region_vector_sort_by_address(vect);
        region_remove_by_address(vect->next, &vect->next->regions[0]);
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
        return Error_No_Memory;
    }
    suffix_size = size - closest->size;
    *address = closest->address;
    region_remove_by_size(containing, closest);
    if (suffix_size != 0) {
        union region_address insertion;
        insertion.virtual = address->virtual + suffix_size;
        assert_ok(region_put_by_size(vect, insertion, suffix_size, 'free'));
    }
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
    region_remove_by_address(containing, closest);
    assert(is_sorted_by_address(vect));
    return size;
}
