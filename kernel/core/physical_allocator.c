#include <external/multiboot.h>
#include <truth/types.h>
#include <truth/panic.h>
#include <truth/physical_allocator.h>
#include <truth/region_vector.h>
#include <truth/memory_sizes.h>

extern struct region_vector init_physical_allocator_vector;

static void insert_regions(struct multiboot_info *multiboot_tables) {
    struct multiboot_mmap_entry *mmap =
        (struct multiboot_mmap_entry *)(uintptr_t)multiboot_tables->mmap_addr;
    for (size_t i = 0; i < multiboot_tables->mmap_length; ++i) {
        if (mmap[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            physical_free(mmap[i].addr, mmap[i].len);
        }
    }
}

void init_physical_allocator(struct multiboot_info *multiboot_tables) {
    init_region_vector(&init_physical_allocator_vector);
    insert_regions(multiboot_tables);
}

phys_addr physical_alloc(size_t pages) {
    union address address;
    size_t size = pages * SMALL_PAGE;
    struct region_vector *vect = &init_physical_allocator_vector;
    if (region_alloc(vect, size, &address) != Ok) {
        return invalid_phys_addr;
    } else {
        return address.physical;
    }
}

void physical_free(phys_addr address, size_t pages) {
    union address in;
    size_t size = pages * SMALL_PAGE;
    in.physical = address;
    struct region_vector *vect = &init_physical_allocator_vector;
    region_free(vect, in, size);
}
