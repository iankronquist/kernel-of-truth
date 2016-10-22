#include <external/multiboot.h>
#include <truth/types.h>
#include <truth/panic.h>
#include <truth/physical_allocator.h>
#include <truth/region_vector.h>
#include <truth/memory_sizes.h>

extern struct region_vector init_physical_allocator_vector;
extern struct region_vector init_physical_used_vector;

#define Boot_Map_Start (phys_addr)0x001000
#define Boot_Map_End   (phys_addr)0x400000

static enum status checked insert_regions(struct multiboot_info *multiboot_tables) {
    union region_address address;
    struct multiboot_mmap_entry *mmap =
        (struct multiboot_mmap_entry *)(uintptr_t)multiboot_tables->mmap_addr;
    for (size_t i = 0; i < multiboot_tables->mmap_length; ++i) {
        if (mmap[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            if (mmap[i].addr + mmap[i].len > Boot_Map_Start &&
                mmap[i].addr < Boot_Map_End) {

                if (Boot_Map_Start > mmap[i].addr) {
                    size_t prefix_length = Boot_Map_Start - mmap[i].addr;
                    address.physical = mmap[i].addr;
                    bubble(region_put_by_size(&init_physical_allocator_vector, address, prefix_length, 'free'), "inserting region prefix");
                }
                if (Boot_Map_End < mmap[i].addr + mmap[i].len) {
                    size_t postfix_length = mmap[i].addr + mmap[i].len -
                                            Boot_Map_End;
                    address.physical = mmap[i].addr + mmap[i].len;
                    bubble(region_put_by_size(&init_physical_allocator_vector, address, postfix_length, 'free'), "inserting region postfix");
                }
            } else {
                log("Non-kernel Block");
                physical_free(mmap[i].addr, 'free');
            }
        }
    }
    log("Contents of physical allocator vector:");
    debug_region_vector(&init_physical_allocator_vector);
    return Ok;
}

enum status checked init_physical_allocator(struct multiboot_info *multiboot_tables) {
    init_region_vector(&init_physical_used_vector);
    init_region_vector(&init_physical_allocator_vector);
    bubble(insert_regions(multiboot_tables),
                          "inserting multiboot map regions");
    return Ok;
}

phys_addr physical_alloc(size_t pages, int tag) {
    union region_address address;
    size_t size = pages * Page_Small;
    enum status status = region_get_by_size(&init_physical_allocator_vector,
                                            size * Page_Small, &address);
    if (status != Ok) {
        return invalid_phys_addr;
    }
    region_put_by_address(&init_physical_used_vector, address, size, tag);
    return address.physical;
}

void physical_free(phys_addr address, int tag) {
    union region_address in;
    in.physical = address;
    size_t size = region_get_by_address(&init_physical_used_vector, in, tag);
    assert_ok(region_put_by_size(&init_physical_allocator_vector, in, size,
                                 tag));
}
