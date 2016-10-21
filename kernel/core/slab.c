#include <arch/x64/paging.h>
#include <truth/panic.h>
#include <truth/slab.h>
#include <truth/types.h>
#include <truth/physical_allocator.h>
#include <truth/region_vector.h>

// The highest lower half canonical address.
#define lower_half_end     ((void *)0x00007fffffffffff)
#define higher_half_start  ((void *)0xffff800000000000)
#define lower_half_               (0x1000000000000)
#define higher_half_ (~0ul - (uintptr_t)higher_half_start)

extern struct region_vector slab_lower_half;
extern struct region_vector slab_higher_half;

static inline bool in_lower_half(void *address) {
    return address <= lower_half_end;
}

void init_slab(void) {
    union address address;
    init_region_vector(&slab_higher_half);
    init_region_vector(&slab_lower_half);
    address.virtual = (void *)(4 * MB);
    region_free(&slab_lower_half, address, lower_half_ - (4 * MB));
    address.virtual = higher_half_start;
    region_free(&slab_higher_half, address, higher_half_);
}

void *slab_alloc(size_t count, enum page_size type,
                 enum memory_attributes page_attributes) {
    union address virt_address;
    union address phys_address;
    struct region_vector *vect;
    if (page_attributes & Memory_User_Access) {
        vect = &slab_lower_half;
    } else {
        vect = &slab_higher_half;
    }
    if (region_alloc(vect, count * type, &virt_address) != Ok) {
        return NULL;
    }
    phys_address.physical = physical_alloc(count * type);
    if (phys_address.physical == invalid_phys_addr) {
        assert(0);
        goto out;
    }
    if (map_page(virt_address.virtual, phys_address.physical,
                 page_attributes) != Ok) {
        physical_free(phys_address.physical, count);
        logf("Failed to map slab page: %p, %lx\n", virt_address.virtual,
             phys_address.physical);
        assert(0);
        goto out;
    }
    return virt_address.virtual;
out:
    region_free(vect, virt_address, count * type);
    return NULL;
}

void slab_free(size_t count, enum page_size type, void *address) {
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
