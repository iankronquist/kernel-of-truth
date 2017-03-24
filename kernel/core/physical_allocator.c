#include <external/multiboot.h>
#include <truth/types.h>
#include <truth/panic.h>
#include <truth/lock.h>
#include <truth/physical_allocator.h>
#include <truth/region_vector.h>
#include <truth/memory.h>


static struct lock physical_allocator_lock = Lock_Clear;

struct physical_page_stack {
    phys_addr next;
};

static volatile struct physical_page_stack *volatile Physical_Page_Stack;
static volatile phys_addr Physical_Page = 0xfff;

#define Boot_Map_Start (phys_addr)0x001000
#define Boot_Map_End   (phys_addr)Kernel_Physical_End

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
                    physical_free_range(mmap[i].addr, prefix_length / Page_Small);
                }
                if (Boot_Map_End < mmap[i].addr + mmap[i].len) {
                    size_t postfix_length = mmap[i].addr + mmap[i].len -
                                            Boot_Map_End;
                    physical_free_range(Boot_Map_End, postfix_length / Page_Small);
                }
            } else {
                physical_free_range(mmap[i].addr, mmap[i].len / Page_Small);
            }
        }
    }
}

void physical_allocator_init(struct multiboot_info *multiboot_tables) {
    Physical_Page_Stack = Kernel_Pivot_Page;
    insert_regions(multiboot_tables);
}

phys_addr physical_alloc(void) {
    phys_addr phys;
    phys_addr next;
    lock_acquire_writer(&physical_allocator_lock);

    phys = Physical_Page;
    next = Physical_Page_Stack->next;
    unmap_page(Physical_Page_Stack, false);
    if (next != invalid_phys_addr) {
        map_page(Physical_Page_Stack, next, Memory_Writable);
    }
    Physical_Page = next;
    assert(phys != next);
    lock_release_writer(&physical_allocator_lock);
    return phys;
}

void physical_free(phys_addr address) {
    assert(is_aligned(address, Page_Small));
    phys_addr prev;
    lock_acquire_writer(&physical_allocator_lock);

    prev = Physical_Page;
    unmap_page(Physical_Page_Stack, false);
    map_page(Physical_Page_Stack, address, Memory_Writable);
    Physical_Page_Stack->next = prev;
    Physical_Page = address;

    lock_release_writer(&physical_allocator_lock);
}


void physical_free_range(phys_addr address, size_t pages) {
    assert(is_aligned(address, Page_Small));
    for (size_t i = 0; i < pages; ++i) {
        physical_free(address);
        address += Page_Small;
    }
}
