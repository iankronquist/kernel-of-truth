#include <arch/x64/paging.h>
#include <truth/panic.h>
#include <truth/slab.h>
#include <truth/types.h>
#include <truth/physical_allocator.h>
#include <truth/region_vector.h>

// The highest lower half canonical address.
#define Lower_Half_End     ((void *)0x00007fffffffffff)
#define Higher_Half_Start  ((void *)0xffff800000000000)
#define Lower_Half_Size               (0x1000000000000)
#define Higher_Half_Size (~0ul - (uintptr_t)Higher_Half_Start)

extern struct region_vector slab_lower_half_used;
extern struct region_vector slab_lower_half_free;
extern struct region_vector slab_higher_half_used;
extern struct region_vector slab_higher_half_free;

static inline bool in_lower_half(void *address) {
    return address <= Lower_Half_End;
}

enum status checked init_slab(void) {
    log("~ 0");
    union region_address address;
    log("~ 1");
    init_region_vector(&slab_higher_half_used);
    init_region_vector(&slab_higher_half_free);
    init_region_vector(&slab_lower_half_used);
    init_region_vector(&slab_lower_half_free);
    log("~ 2");
    address.virtual = NULL;
    size_t initial_mapping_size = 4 * MB;
    bubble(region_put_by_address(&slab_lower_half_used, address, initial_mapping_size, 'boot'), "initialize slab boot memory");
    log("~ 3");
    bubble(region_put_by_size(&slab_lower_half_free, address, Lower_Half_Size - initial_mapping_size, 'free'), "initialize slab lower half");
    log("~ 4");
    address.virtual = Higher_Half_Start;
    bubble(region_put_by_size(&slab_higher_half_free, address, Higher_Half_Size, 'free'), "initialize slab higher half");
    log("~ 5");
    address.virtual = Higher_Half_Start;
    return Ok;
}


void *slab_alloc(size_t bytes, enum memory_attributes page_attributes, int tag) {
    if (!is_aligned(bytes, Page_Small)) {
        return NULL;
    }
    union region_address virt_address;
    union region_address phys_address;
    struct region_vector *vect_free;
    struct region_vector *vect_used;
    if (page_attributes & Memory_User_Access) {
        vect_free = &slab_lower_half_free;
        vect_used = &slab_lower_half_used;
    } else {
        vect_free = &slab_higher_half_free;
        vect_used = &slab_higher_half_used;
    }
    if (region_get_by_size(vect_free, bytes, &virt_address) != Ok) {
        return NULL;
    }
    assert_ok(region_put_by_address(vect_used, virt_address, bytes, tag));
    phys_address.physical = physical_alloc(bytes / Page_Small, tag);
    if (phys_address.physical == invalid_phys_addr) {
        logf("%lx\n", bytes);
        assert(0);
        goto out;
    }
    if (map_page(virt_address.virtual, phys_address.physical,
                 page_attributes) != Ok) {
        assert(0);
        physical_free(phys_address.physical, tag);
        goto out;
    }
    return virt_address.virtual;
out:
    region_get_by_address(vect_used, virt_address, tag);
    assert_ok(region_put_by_size(vect_free, virt_address, bytes, 'free'));
    return NULL;
}

void slab_free(void *address, int tag) {
    union region_address virt_address;
    struct region_vector *vect_free;
    struct region_vector *vect_used;
    virt_address.virtual = address;
    if (in_lower_half(address)) {
        vect_free = &slab_lower_half_free;
        vect_used = &slab_lower_half_used;
    } else {
        vect_free = &slab_higher_half_free;
        vect_used = &slab_higher_half_used;
    }
    size_t size = region_get_by_address(vect_used, virt_address, tag);
    assert_ok(region_put_by_size(vect_free, virt_address, size, 'free'));
}
