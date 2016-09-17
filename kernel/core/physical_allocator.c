#include <arch/x64/paging.h>
#include <external/multiboot.h>
#include <truth/types.h>
#include <truth/panic.h>
#include <truth/physical_allocator.h>

// FIXME this macro is used elswhere too
#define SMALL_PAGE 0x1000

struct physical_region {
    phys_addr address;
    size_t size;
};

struct physical_region_vector {
    size_t regions_used;
    struct physical_region_vector *next;
    struct physical_region regions[];
};

#define regions_count ( \
        (SMALL_PAGE - sizeof(struct physical_region_vector)) / \
        sizeof(struct physical_region) \
        )

extern struct physical_region_vector init_physical_allocator_vector;

static void insert_regions(struct multiboot_info *multiboot_tables) {
    struct multiboot_mmap_entry *mmap =
        (struct multiboot_mmap_entry*)(uintptr_t)multiboot_tables->mmap_addr;
    for (size_t i = 0; i < multiboot_tables->mmap_length; ++i) {
        if (mmap[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            physical_free(mmap[i].addr, mmap[i].len);
        }
    }
}

void init_physical_allocator(struct multiboot_info *multiboot_tables) {
    init_physical_allocator_vector.regions_used = 0;
    init_physical_allocator_vector.next = NULL;
    enum status unused(status) = map_page(multiboot_tables, (phys_addr)multiboot_tables, page_none);
    insert_regions(multiboot_tables);
}

phys_addr physical_alloc(size_t pages) {
    size_t size = pages * SMALL_PAGE;
    struct physical_region_vector *vect = &init_physical_allocator_vector;
    do {
        for (size_t i = 0; i < vect->regions_used && i < regions_count; ++i) {
            if (vect->regions[i].size > size) {
                phys_addr address = vect->regions[i].address;
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
                return address;
            }
        }
    } while (vect != NULL);
    return invalid_phys_addr;
}

void physical_free(phys_addr address, size_t pages) {
    size_t size = pages * SMALL_PAGE;
    //struct physical_region_vector *prev = NULL;
    struct physical_region_vector *vect = &init_physical_allocator_vector;
    do {
        if (vect->regions_used != regions_count) {
            vect->regions[vect->regions_used].address = address;
            vect->regions[vect->regions_used].size = size;
            vect->regions_used++;
            return;
        }
        //prev = vect;
        vect = vect->next;
    } while (vect != NULL);
    // Not implemented.
    /*
       struct physical_region_vector *new = slab_alloc(1, slab_small,
       slab_higher_half);
       assert(new != NULL);
       prev->next = new;
       new->next = NULL;
       new->regions_used = 1;
       new->regions[0].size = size;
       new->regions[0].address = address;
     */
}
