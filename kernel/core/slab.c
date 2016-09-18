#include <arch/x64/paging.h>
#include <truth/slab.h>
#include <truth/types.h>
#include <truth/physical_allocator.h>
#include <truth/region_vector.h>

// The highest lower half canonical address.
#define lower_half_end ((void *)0xffffffffffff)

extern struct region_vector slab_lower_half;
extern struct region_vector slab_higher_half;

static inline bool in_lower_half(void *address) {
    return address <= lower_half_end;
}

void init_slab(void) {
    init_region_vector(&slab_higher_half);
    init_region_vector(&slab_lower_half);
}

void*slab_alloc(size_t count, enum slab_type type,
                enum slab_attributes attrs,
                enum page_attributes page_attributes) {
    union address virt_address;
    union address phys_address;
    struct region_vector *vect;
    if (attrs == slab_kernel_memory) {
        vect = &slab_lower_half;
        page_attributes = page_attributes | page_user_access;
    } else {
        vect = &slab_higher_half;
        page_attributes = page_attributes;
    }
    if (region_alloc(vect, count * type, &virt_address) != Ok) {
        return NULL;
    }
    phys_address.physical = physical_alloc(count * type);
    if (phys_address.physical == invalid_phys_addr) {
        goto out;
    }
    if (map_page(virt_address.virtual, phys_address.physical,
                 page_attributes) != Ok) {
        physical_free(count * type, phys_address.physical);
        goto out;
    }
    return virt_address.virtual;
out:
    region_free(vect, virt_address, count * type);
    return NULL;
}

void slab_free(size_t count, enum slab_type type, void *address) {
    union address virt_address;
    struct region_vector *vect;
    virt_address.virtual = address;
    if (in_lower_half(address)) {
        vect = &slab_lower_half;
    } else {
        vect = &slab_higher_half;
    }
    region_free(vect, virt_address, type * count);
}
