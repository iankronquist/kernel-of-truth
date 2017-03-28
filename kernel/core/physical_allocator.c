#include <arch/x64/paging.h>
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

static struct physical_page_stack *const Physical_Page_Stack =
    Kernel_Pivot_Page;
static volatile phys_addr Physical_Page = 0xfff;

static void physical_free_range(phys_addr address, size_t pages);

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
        if (map_page(Physical_Page_Stack, next, Memory_Writable) != Ok) {
            phys = invalid_phys_addr;
            goto out;
        }
    }
    Physical_Page = next;
    assert(phys != next);
out:
    lock_release_writer(&physical_allocator_lock);
    return phys;
}

void physical_free(phys_addr address) {
    assert(is_aligned(address, Page_Small));
    phys_addr prev;
    lock_acquire_writer(&physical_allocator_lock);

    prev = Physical_Page;
    unmap_page(Physical_Page_Stack, false);
    assert_ok(map_page(Physical_Page_Stack, address, Memory_Writable));
    Physical_Page_Stack->next = prev;
    Physical_Page = address;

    lock_release_writer(&physical_allocator_lock);
}


enum status physical_page_remove(phys_addr address) {
    enum status status = Error_Absent;
    assert(is_aligned(address, Page_Small));
    phys_addr original = Physical_Page;
    phys_addr current = original;
    lock_acquire_writer(&physical_allocator_lock);
    logf(Log_Debug, "aa-1\n");
    while (current != invalid_phys_addr) {
        logf(Log_Debug, "%lx %lx\n", current, Physical_Page_Stack->next);
        unmap_page(Physical_Page_Stack, false);
        assert_ok(map_page(Physical_Page_Stack, current, Memory_Writable));
        current = Physical_Page_Stack->next;

        if (Physical_Page_Stack->next == address) {
            unmap_page(Physical_Page_Stack, false);
            assert_ok(map_page(Physical_Page_Stack, Physical_Page_Stack->next,
                     Memory_Writable));
            phys_addr next_next = Physical_Page_Stack->next;
            unmap_page(Physical_Page_Stack, false);
            assert_ok(map_page(Physical_Page_Stack, current, Memory_Writable));
            Physical_Page_Stack->next = next_next;
            status = Ok;
            break;
        }
    }
    logf(Log_Debug, "aa0\n");
    unmap_page(Physical_Page_Stack, false);
    assert_ok(map_page(Physical_Page_Stack, original, Memory_Writable));
    logf(Log_Debug, "aa1\n");
    lock_release_writer(&physical_allocator_lock);
    logf(Log_Debug, "aa2 %s\n", status_message(status));
    return status;
}


static void physical_free_range(phys_addr address, size_t pages) {
    assert(is_aligned(address, Page_Small));
    for (size_t i = 0; i < pages; ++i) {
        physical_free(address);
        address += Page_Small;
    }
}
