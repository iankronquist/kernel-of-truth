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

size_t Usage = 0;

size_t slab_get_usage(void) {
    return Usage;
}


void *slab_alloc_request_physical(phys_addr phys, size_t size,
                                  enum memory_attributes attrs) {
    enum status status;
    void *virt_address;
    if (!is_aligned(size, Page_Small)) {
        return NULL;
    } else if (size == 0) {
        return NULL;
    }

    lock_acquire_writer(&slab_higher_half_lock);

    if (region_alloc(&slab_higher_half, size, &virt_address) != Ok) {
        virt_address = NULL;
        goto out;
    }

    status = map_range(virt_address, phys, size / Page_Small, attrs);
    if (status != Ok) {
        region_free(&slab_higher_half, virt_address, size);
        virt_address = NULL;
        goto out;
    }

    Usage -= size;
out:
    lock_release_writer(&slab_higher_half_lock);

    return virt_address;
}


void *slab_alloc_request_physical_random(phys_addr phys, size_t size, enum memory_attributes attrs) {
    enum status status;
    void *virt_address;
    if (!is_aligned(size, Page_Small)) {
        return NULL;
    } else if (size == 0) {
        return NULL;
    }

    lock_acquire_writer(&slab_higher_half_lock);

    if (region_alloc_random(&slab_higher_half, size, &virt_address) != Ok) {
        virt_address = NULL;
        goto out;
    }

    status = map_range(virt_address, phys, size / Page_Small, attrs);
    if (status != Ok) {
        region_free(&slab_higher_half, virt_address, size);
        virt_address = NULL;
        goto out;
    }

    Usage -= size;
out:
    lock_release_writer(&slab_higher_half_lock);

    return virt_address;
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
    void *virt_address;
    void *next_virt_address;
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
        if (map_page(next_virt_address, phys_address,
                     page_attributes) != Ok) {
            unmap_range(virt_address, i, true);
            logf(Log_Warning, "Failed to map slab page: %p, %lx\n",
                 next_virt_address, phys_address);
            assert(0);
            goto out;
        }
        next_virt_address += Page_Small;
    }
    Usage -= bytes;
    return virt_address;
out:
    region_free(vect, virt_address, bytes / Page_Small);
    return NULL;
}


void slab_free_helper(size_t bytes, void *address, struct region_vector *vect,
                      bool free_phys) {
    if (address == NULL) {
        return;
    }
    void *virt_address;
    virt_address  = address;
    unmap_range(virt_address, bytes / Page_Small, free_phys);
    Usage += bytes;
    region_free(vect, virt_address, bytes);
}


void slab_init(void) {
    void *address;
    region_vector_init(&slab_higher_half);
    address = Kernel_Virtual_End;
    Usage += Kernel_Virtual_Start - Kernel_Memory_Start;
    region_free(&slab_higher_half, address, Kernel_Virtual_Start -
                Kernel_Memory_Start);
    Usage += Higher_Half_End - Kernel_Virtual_End;
    region_free(&slab_higher_half, address, Higher_Half_End -
                Kernel_Virtual_End);
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
