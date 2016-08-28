#include <arch/x86/process.h>

#include <truth/kassert.h>
#include <truth/klog.h>
#include <truth/lock.h>
#include <truth/pmem.h>
#include <truth/types.h>
#include <truth/vmem.h>

#include <truth/private/memlayout.h>
#include <truth/private/region.h>

static phys_region_t *physical_memory;
static spinlock_t physical_memory_lock = SPINLOCK_INIT;

static inline phys_region_t *get_cur_free_regions(void) {
    return physical_memory;
}

void physical_allocator_init(struct multiboot_info *mb) {
    kassert(init_phys_allocator(mb, 0) == Ok);
}

status_t checked init_phys_allocator(struct multiboot_info *mb,
        page_frame_t *unused(highest_address)) {

    acquire_spinlock(&physical_memory_lock);

    physical_memory = init_region_list();

    multiboot_memory_map_t *mmap = (void*)mb->mmap_addr;
    for (size_t i = 0;
            i < mb->mmap_length/sizeof(multiboot_memory_map_t);
            ++i) {
        if (mmap[i].type == MULTIBOOT_MEMORY_AVAILABLE && (mmap[i].addr >> 32) == 0) {
            uintptr_t addr_low = (uintptr_t)mmap[i].addr;
            uintptr_t length_low = (uintptr_t)mmap[i].len;
            klogf("size %p len %p type %p\n", addr_low, length_low,
                    mmap[i].type);
            status_t stat =
                insert_region((void*)(uintptr_t)(addr_low),
                    ROUND_NEXT_PAGE(length_low), physical_memory);
            if (stat != Ok) {
                destroy_free_list(physical_memory);
                klog("error inserting region");
                return stat;
            }
        }

    }
    klog("Memory from multiboot map");
    debug_region(physical_memory);

    status_t stat = remove_region((void*)KERNEL_START,
            ROUND_NEXT_PAGE(KERNEL_SIZE), physical_memory);
    if (stat != Ok) {
        return stat;
    }

    stat = remove_region((void*)KHEAP_PHYS_ROOT, KHEAP_PHYS_SIZE,
            physical_memory);
    if (stat != Ok) {
        return stat;
    }
    klog("Memory after culling");
    debug_region(physical_memory);

    release_spinlock(&physical_memory_lock);
    return Ok;
}

// Find an available range of physical memory.
page_frame_t get_phys_region(size_t pages) {
    acquire_spinlock(&physical_memory_lock);
    page_frame_t addr = (page_frame_t)find_region(pages,
            get_cur_free_regions());
    release_spinlock(&physical_memory_lock);
    klogf("Getting region %p of size %p\n", addr, pages);
    klogf("%p paligned? \n", addr);
    kassert(PAGE_ALIGN(addr) == addr);
    return addr;
}

// Return a range of physical memory to the pool.
void put_phys_region(page_frame_t region, size_t pages) {
    kassert(PAGE_ALIGN(region) == region);
    acquire_spinlock(&physical_memory_lock);
    status_t stat = insert_region((void*)region, pages,
            get_cur_free_regions());
    kassert(stat != Err);
    release_spinlock(&physical_memory_lock);
    klogf("Putting region %p of size %p\n", region, pages);
}

// Get a single page.
page_frame_t alloc_frame(void) {
    return get_phys_region(PAGE_SIZE);
}

// Return a single page to the pool.
void free_frame(page_frame_t frame) {
    put_phys_region(frame, PAGE_SIZE);
}
