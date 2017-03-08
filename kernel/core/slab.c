#include <arch/x64/paging.h>
#include <truth/panic.h>
#include <truth/slab.h>
#include <truth/lock.h>
#include <truth/types.h>
#include <truth/physical_allocator.h>
#include <truth/region_vector.h>

// The highest lower half canonical address.
#define lower_half_start  ((void *)0)
#define lower_half_end    ((void *)0x00007fffffffffff)
#define higher_half_start ((void *)0xffff800000000000)
#define higher_half_end   ((void *)~0)
#define lower_half_size   (lower_half_end - lower_half_start)
#define higher_half_size  (higher_half_end - higher_half_start)

static struct lock slab_lower_half_lock = Lock_Clear;
static struct lock slab_higher_half_lock = Lock_Clear;

extern struct region_vector slab_lower_half;
extern struct region_vector slab_higher_half;


static inline bool in_lower_half(void *address) {
    return address <= lower_half_end;
}


static void *slab_alloc_helper(size_t bytes, phys_addr *phys,
                               enum memory_attributes page_attributes,
                               struct region_vector *vect) {
    if (!is_aligned(bytes, Page_Small)) {
        logf(Log_Error, "unaligned %lx %x\n", bytes, Page_Small);
        return NULL;
    }
    union address virt_address;
    union address phys_address;
    if (region_alloc(vect, bytes, &virt_address) != Ok) {
        return NULL;
    }
    phys_address.physical = physical_alloc(bytes / Page_Small);
    *phys = phys_address.physical;
    if (phys_address.physical == invalid_phys_addr) {
        assert(0);
        goto out;
    }
    if (map_page(virt_address.virtual, phys_address.physical,
                 page_attributes) != Ok) {
        physical_free(phys_address.physical, bytes / Page_Small);
        logf(Log_Warning, "Failed to map slab page: %p, %lx\n",
             virt_address.virtual, phys_address.physical);
        assert(0);
        goto out;
    }
    return virt_address.virtual;
out:
    region_free(vect, virt_address, bytes / Page_Small);
    return NULL;
}


static void slab_free_helper(size_t bytes, void *address,
                             struct region_vector *vect) {
    union address virt_address;
    virt_address.virtual = address;
    unmap_page(virt_address.virtual, true);
    region_free(vect, virt_address, bytes);
}


void slab_init(void) {
    union address address;
    region_vector_init(&slab_higher_half);
    region_vector_init(&slab_lower_half);
    address.virtual = (void *)(4 * MB);
    region_free(&slab_lower_half, address, lower_half_size - (4 * MB));
    address.virtual = higher_half_start;
    region_free(&slab_higher_half, address, higher_half_size);
}


void slab_free(size_t bytes, void *address) {
    if (in_lower_half(address)) {
        lock_acquire_writer(&slab_lower_half_lock);
        slab_free_helper(bytes, address, &slab_lower_half);
        lock_release_writer(&slab_lower_half_lock);
    } else {
        lock_acquire_writer(&slab_higher_half_lock);
        slab_free_helper(bytes, address, &slab_higher_half);
        lock_release_writer(&slab_higher_half_lock);
    }
}


void slab_free_locked(size_t bytes, void *address) {
    if (in_lower_half(address)) {
        slab_free_helper(bytes, address, &slab_lower_half);
    } else {
        slab_free_helper(bytes, address, &slab_higher_half);
    }
}


void *slab_alloc_locked(size_t bytes, enum memory_attributes page_attributes) {
    phys_addr phys;
    void *virt;

    if (page_attributes & Memory_User_Access) {
        virt = slab_alloc_helper(bytes, &phys, page_attributes,
                                 &slab_lower_half);
    } else {
        virt = slab_alloc_helper(bytes, &phys, page_attributes,
                                 &slab_higher_half);
    }

    return virt;
}



void *slab_alloc(size_t bytes, enum memory_attributes page_attributes) {
    phys_addr phys;
    void *virt;

    if (page_attributes & Memory_User_Access) {
        lock_acquire_writer(&slab_lower_half_lock);
        virt = slab_alloc_helper(bytes, &phys, page_attributes,
                                 &slab_lower_half);
        lock_release_writer(&slab_lower_half_lock);
    } else {
        lock_acquire_writer(&slab_higher_half_lock);
        virt = slab_alloc_helper(bytes, &phys, page_attributes,
                                 &slab_higher_half);
        lock_release_writer(&slab_higher_half_lock);
    }

    return virt;
}


void *slab_alloc_phys(phys_addr *phys, enum memory_attributes page_attributes) {
    void *virt;
    if (page_attributes & Memory_User_Access) {
        lock_acquire_writer(&slab_lower_half_lock);
        virt = slab_alloc_helper(Page_Small, phys, page_attributes,
                                 &slab_lower_half);
        lock_release_writer(&slab_lower_half_lock);
    } else {
        lock_acquire_writer(&slab_higher_half_lock);
        virt = slab_alloc_helper(Page_Small, phys, page_attributes,
                                 &slab_higher_half);
        lock_release_writer(&slab_higher_half_lock);
    }
    return virt;
}
