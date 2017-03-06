#include <external/multiboot.h>
#include <truth/types.h>
#include <truth/panic.h>
#include <truth/physical_allocator.h>
#include <truth/region_vector.h>
#include <truth/memory.h>

extern struct region_vector init_physical_allocator_vector;

#define Boot_Map_Start (phys_addr)0x001000
#define Boot_Map_End   (phys_addr)0x400000

static void insert_regions(struct multiboot_info *multiboot_tables) {
    struct multiboot_mmap_entry *mmap =
        (struct multiboot_mmap_entry *)(uintptr_t)multiboot_tables->mmap_addr;
    uint64_t total = 0;
    for (size_t i = 0;
            i < multiboot_tables->mmap_length /
                    sizeof(struct multiboot_mmap_entry);
            ++i) {
        if (mmap[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            total += mmap[i].len;
            if (mmap[i].addr + mmap[i].len > Boot_Map_Start &&
                mmap[i].addr < Boot_Map_End) {

                if (Boot_Map_Start > mmap[i].addr) {
                    size_t prefix_length = Boot_Map_Start - mmap[i].addr;
                    physical_free(mmap[i].addr, prefix_length / Page_Small);
                }
                if (Boot_Map_End < mmap[i].addr + mmap[i].len) {
                    size_t postfix_length = mmap[i].addr + mmap[i].len -
                                            Boot_Map_End;
                    physical_free(Boot_Map_End, postfix_length / Page_Small);
                }
            } else {
                physical_free(mmap[i].addr, mmap[i].len / Page_Small);
            }
        }
    }
    log(Log_Debug, "Contents of physical allocator vector:");
    debug_region_vector(&init_physical_allocator_vector);
}

void debug_physical_allocator(void) {
    debug_region_vector(&init_physical_allocator_vector);
}

void physical_allocator_init(struct multiboot_info *multiboot_tables) {
    region_vector_init(&init_physical_allocator_vector);
    insert_regions(multiboot_tables);
}

phys_addr physical_alloc(size_t pages) {
    union address address;
    size_t size = pages * Page_Small;
    struct region_vector *vect = &init_physical_allocator_vector;
    if (region_alloc(vect, size, &address) != Ok) {
        return invalid_phys_addr;
    } else {
        return address.physical;
    }
}

void physical_free(phys_addr address, size_t pages) {
    union address in;
    size_t size = pages * Page_Small;
    in.physical = address;
    struct region_vector *vect = &init_physical_allocator_vector;
    region_free(vect, in, size);
}
