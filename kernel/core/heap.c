#include <truth/panic.h>
#include <truth/region_vector.h>
#include <truth/slab.h>
#include <truth/string.h>
#include <truth/types.h>

#define heap_redzone_size sizeof(long)
#define heap_redzone_fill 0xccccccccu

struct region_vector *heap_metadata_used;
struct region_vector *heap_metadata_free;
byte *heap = NULL;

enum status checked init_heap(void) {
    union address heap_address;
    heap_address.virtual = heap;
    heap_metadata_used = slab_alloc(1, slab_small, slab_kernel_memory,
                                    page_writable);
    if (heap_metadata_used == NULL) {
        assert(0);
        return Error_No_Memory;
    }
    heap_metadata_free = slab_alloc(1, slab_small, slab_kernel_memory,
                                    page_writable);
    if (heap_metadata_free == NULL) {
        slab_free(1, slab_small, heap_metadata_used);
        assert(0);
        return Error_No_Memory;
    }
    heap = slab_alloc(1, slab_small, slab_kernel_memory, page_writable);
    if (heap == NULL) {
        slab_free(1, slab_small, heap_metadata_used);
        slab_free(1, slab_small, heap_metadata_free);
        assert(0);
        return Error_No_Memory;
    }

    heap_metadata_used = slab_alloc(1, slab_small, slab_user_memory, page_writable);
    init_region_vector(heap_metadata_used);
    heap_metadata_free = slab_alloc(1, slab_small, slab_user_memory, page_writable);
    init_region_vector(heap_metadata_free);
    region_free(heap_metadata_free, heap_address, SMALL_PAGE);
    memset(heap, heap_redzone_fill, SMALL_PAGE);
    return Ok;
}

void *kmalloc(size_t bytes) {
    union address address;
    size_t allocation_size = bytes + 2 * heap_redzone_size;

    enum status status = region_alloc(heap_metadata_free,
                                      allocation_size, &address);
    if (status != Ok) {
        return NULL;
    }
    region_free(heap_metadata_used, address, allocation_size);
    unsigned long *redzone_prefix = address.virtual;
    unsigned long *redzone_suffix =
        (unsigned long *)(address.bytes + bytes + heap_redzone_size);
    assert(*redzone_prefix == heap_redzone_fill &&
           *redzone_suffix == heap_redzone_fill);
    return address.bytes + heap_redzone_size;
}

void *kcalloc(size_t count, size_t size) {
    void *address = kmalloc(count * size);
    memset(address, 0, count * size);
    return address;
}

void kfree(void *address) {
    union address addr;
    addr.virtual = address;
    size_t size = region_find_size_and_free(heap_metadata_used, addr);
    region_free(heap_metadata_free, addr, size);
}
