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

static bool physical_region_contains(uintptr_t start_a, uintptr_t end_a,
                                     uintptr_t start_b, uintptr_t end_b) {
    return (start_b > start_a && start_b < end_a) ||
           (end_b > start_a && end_b < end_a);
}

// When populating the physical allocator, certain addresses are unavailable or
// reserved. The multiboot tables give a sorted list of regions which are
// usable memory, but some of these contain the kernel, modules, or the
// multiboot tables themselves.
static void insert_regions(struct multiboot_info *multiboot_tables) {
    struct reserved_region {
        phys_addr start;
        phys_addr end;
    };
    enum status status;
    size_t modules_index_start;
    size_t reserved_count;
    bool inserted = false;
    uint64_t total = 0;

    struct multiboot_mod_list *modules =
        (struct multiboot_mod_list *)(uintptr_t)multiboot_tables->mods_addr;
    struct multiboot_mmap_entry *mmap =
        (struct multiboot_mmap_entry *)(uintptr_t)multiboot_tables->mmap_addr;


    uintptr_t mods_list_start = align_as(multiboot_tables->mods_addr,
                                         Page_Small);
    uintptr_t mods_list_end = round_next(multiboot_tables->mods_addr +
                                         multiboot_tables->mods_count *
                                         sizeof(struct multiboot_mod_list),
                                         Page_Small);

    if (physical_region_contains(Boot_Map_Start, Boot_Map_End,
                                 mods_list_start, mods_list_end)) {
        modules_index_start = 1;
        reserved_count = multiboot_tables->mods_count + 1;
    } else {
        modules_index_start = 2;
        reserved_count = multiboot_tables->mods_count + 2;

        status = map_range(modules, mods_list_start,
                           (mods_list_end - mods_list_start) / Page_Small,
                           Memory_No_Attributes);
        assert_ok(status);
    }

    struct reserved_region reserved[reserved_count];

    if (modules_index_start == 2) {
        if (Boot_Map_Start < mods_list_start) {
            reserved[0].start = Boot_Map_Start;
            reserved[0].end = Boot_Map_End;
            reserved[1].start = mods_list_start;
            reserved[1].end = mods_list_end;
        } else {
            reserved[0].start = mods_list_start;
            reserved[0].end = mods_list_end;
            reserved[1].start = Boot_Map_Start;
            reserved[1].end = Boot_Map_End;
        }
    } else {
        reserved[0].start = Boot_Map_Start;
        reserved[0].end = Boot_Map_End;
    }

    for (size_t i = 0;
         i < multiboot_tables->mods_count; ++i) {

        reserved[i + modules_index_start].start = modules[i].mod_start;
        reserved[i + modules_index_start].end = round_next(modules[i].mod_end,
                                                           Page_Small);
    }

    for (size_t i = 0; i < (multiboot_tables->mmap_length /
                            sizeof(struct multiboot_mmap_entry)); ++i) {
        inserted = false;
        if (mmap[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            total += mmap[i].len;
            for (size_t j = 0; j < reserved_count; ++j) {
                uintptr_t mem_start = mmap[i].addr;
                uintptr_t mem_end = mmap[i].addr + mmap[i].len;
                if (physical_region_contains(mem_start, mem_end,
                                             reserved[j].start,
                                             reserved[j].end)) {
                    inserted = true;
                    if (reserved[j].start > mem_start) {
                        physical_free_range(mem_start,
                                            reserved[j].start - mem_start);
                    }
                    if (j + 1 < reserved_count &&
                            physical_region_contains(mem_start, mem_end,
                                                     reserved[j + 1].start,
                                                     reserved[j + 1].end)) {
                        if (reserved[j].end != reserved[j + 1].start) {
                            physical_free_range(reserved[j].end,
                                                reserved[j + 1].start -
                                                reserved[j].end);
                        }
                        mmap[i].addr = reserved[j + 1].start;
                        mmap[i].len -= reserved[j + 1].start -
                                       reserved[j].start;
                    } else if (reserved[j].end < mem_end) {
                        physical_free_range(reserved[j].end,
                                            mem_end - reserved[j].end);
                    }
                }
            }
            if (!inserted) {
                physical_free_range(mmap[i].addr,
                                    mmap[i].len);
            }
        }
    }

    if (modules_index_start == 2) {
        unmap_range(modules, (mods_list_end - mods_list_start) / Page_Small,
                    false);
    }
}

void physical_allocator_init(struct multiboot_info *multiboot_tables) {
    insert_regions(multiboot_tables);
}

// FIXME: x86_64-elf-gcc 6.1.0 with -O2 and UBSAN interact poorly here.
// UBSAN always reports:
// `store to address Physical_Page_Stack->next with insufficient space for
// object of type 'phys_addr'`
// As a fix until I figure out something better, disable all optimizations
// here.
#pragma GCC push_options
#pragma GCC optimize ("O0")
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
#pragma GCC pop_options

void physical_stack_debug(void) {
    phys_addr original = Physical_Page;
    phys_addr current = Physical_Page;
    lock_acquire_writer(&physical_allocator_lock);
    while (current != invalid_phys_addr) {
        unmap_page(Physical_Page_Stack, false);
        assert_ok(map_page(Physical_Page_Stack, current, Memory_Writable));
        logf(Log_Debug, "%lx\n", current);
        current = Physical_Page_Stack->next;
    }
    logf(Log_Debug, "%lx\n", current);
    unmap_page(Physical_Page_Stack, false);
    assert_ok(map_page(Physical_Page_Stack, original, Memory_Writable));
    lock_release_writer(&physical_allocator_lock);
}



enum status physical_page_remove(phys_addr address) {
    enum status status = Error_Absent;
    assert(is_aligned(address, Page_Small));
    phys_addr original = Physical_Page;
    phys_addr current = original;
    lock_acquire_writer(&physical_allocator_lock);
    while (current != invalid_phys_addr) {
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
    unmap_page(Physical_Page_Stack, false);
    assert_ok(map_page(Physical_Page_Stack, original, Memory_Writable));
    lock_release_writer(&physical_allocator_lock);
    return status;
}

static void physical_free_range(phys_addr address, size_t length) {
    assert(length > 0);
    assert(is_aligned(address, Page_Small));
    for (size_t i = 0; i < length / Page_Small; ++i) {
        physical_free(address);
        address += Page_Small;
    }
}
