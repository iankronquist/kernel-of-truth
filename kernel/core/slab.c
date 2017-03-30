#include <arch/x64/paging.h>
#include <truth/panic.h>
#include <truth/slab.h>
#include <truth/lock.h>
#include <truth/types.h>
#include <truth/physical_allocator.h>
#include <truth/region_vector.h>
#include <truth/memory.h>

static struct lock slab_higher_half_lock = Lock_Clear;

extern struct region_vector slab_higher_half;


void *slab_alloc_request_physical(phys_addr phys,
                                  enum memory_attributes attrs) {
    union address virt_address;

    lock_acquire_writer(&slab_higher_half_lock);
    if (region_alloc(&slab_higher_half, Page_Small, &virt_address) != Ok) {
        virt_address.virtual = NULL;
        goto out;
    }

    if (map_page(virt_address.virtual, phys, attrs) != Ok) {
        virt_address.virtual = NULL;
        goto out;
    }

out:
    lock_release_writer(&slab_higher_half_lock);

    return virt_address.virtual;
}


void *slab_alloc_helper(size_t bytes, phys_addr *phys,
                        enum memory_attributes page_attributes,
                        struct region_vector *vect) {
    assert(vect != NULL);
    assert(phys != NULL);
    if (!is_aligned(bytes, Page_Small)) {
        logf(Log_Error, "unaligned %lx %x\n", bytes, Page_Small);
        return NULL;
    }
    union address virt_address;
    union address next_virt_address;
    phys_addr phys_address;
    if (region_alloc(vect, bytes, &virt_address) != Ok) {
        return NULL;
    }
    next_virt_address = virt_address;
    for (size_t i = 0; i < bytes / Page_Small; ++i) {
        phys_address = physical_alloc();
        *phys = phys_address;
        if (phys_address== invalid_phys_addr) {
            assert(0);
            goto out;
        }
        if (map_page(next_virt_address.virtual, phys_address,
                     page_attributes) != Ok) {
            for (size_t j = 0; j < i; ++j) {
                unmap_page(next_virt_address.virtual, true);
                next_virt_address.virtual -= Page_Small;
            }
            logf(Log_Warning, "Failed to map slab page: %p, %lx\n",
                 next_virt_address.virtual, phys_address);
            assert(0);
            goto out;
        }
        next_virt_address.virtual += Page_Small;
    }
    return virt_address.virtual;
out:
    region_free(vect, virt_address, bytes / Page_Small);
    return NULL;
}


void slab_free_helper(size_t bytes, void *address, struct region_vector *vect,
                      bool free_phys) {
    if (address == NULL) {
        return;
    }
    union address virt_address;
    virt_address.virtual = address;
    unmap_page(virt_address.virtual, free_phys);
    region_free(vect, virt_address, bytes);
}


void slab_init(void) {
    union address address;
    region_vector_init(&slab_higher_half);
    address.virtual = Kernel_Virtual_End;
    region_free(&slab_higher_half, address, Higher_Half_Size - Kernel_Image_Size - (4 * MB));
}


void slab_free(size_t bytes, void *address) {
    assert(!memory_is_lower_half(address));
    lock_acquire_writer(&slab_higher_half_lock);
    slab_free_helper(bytes, address, &slab_higher_half, true);
    lock_release_writer(&slab_higher_half_lock);
}


void slab_free_virt(size_t bytes, void *address) {
    assert(!memory_is_lower_half(address));
    lock_acquire_writer(&slab_higher_half_lock);
    slab_free_helper(bytes, address, &slab_higher_half, false);
    lock_release_writer(&slab_higher_half_lock);
}


void slab_free_locked(size_t bytes, void *address) {
    assert(!memory_is_lower_half(address));
    slab_free_helper(bytes, address, &slab_higher_half, true);
}


void *slab_alloc_locked(size_t bytes, enum memory_attributes page_attributes) {
    phys_addr phys;
    void *virt;

    assert(!(page_attributes & Memory_User_Access));

    virt = slab_alloc_helper(bytes, &phys, page_attributes,
                             &slab_higher_half);

    return virt;
}



void *slab_alloc(size_t bytes, enum memory_attributes page_attributes) {
    phys_addr phys;
    void *virt;

    assert(!(page_attributes & Memory_User_Access));

    lock_acquire_writer(&slab_higher_half_lock);
    virt = slab_alloc_helper(bytes, &phys, page_attributes, &slab_higher_half);
    lock_release_writer(&slab_higher_half_lock);

    return virt;
}


void *slab_alloc_phys(phys_addr *phys, enum memory_attributes page_attributes) {
    void *virt;

    assert(!(page_attributes & Memory_User_Access));

    lock_acquire_writer(&slab_higher_half_lock);
    virt = slab_alloc_helper(Page_Small, phys, page_attributes,
                             &slab_higher_half);
    lock_release_writer(&slab_higher_half_lock);
    return virt;
}
