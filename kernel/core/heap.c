#include <truth/memory.h>
#include <truth/panic.h>
#include <truth/region_vector.h>
#include <truth/slab.h>
#include <truth/string.h>
#include <truth/types.h>

#define Heap_Red_Zone_Size sizeof(unsigned long)
#define Heap_Red_Zone_Fill 0xccccccccccccccccul

#define Heap_Size (16 * Page_Small)

struct region_vector *heap_metadata_used;
struct region_vector *heap_metadata_free;
uint8_t *heap = NULL;

enum status checked heap_init(void) {
    union address heap_address;
    heap_metadata_used = slab_alloc(Page_Small, Memory_Writable);
    if (heap_metadata_used == NULL) {
        assert(0);
        return Error_No_Memory;
    }
    heap_metadata_free = slab_alloc(Page_Small, Memory_Writable);
    if (heap_metadata_free == NULL) {
        slab_free(Page_Small, heap_metadata_used);
        assert(0);
        return Error_No_Memory;
    }
    heap = slab_alloc(Heap_Size, Memory_Writable);
    if (heap == NULL) {
        slab_free(Page_Small, heap_metadata_used);
        slab_free(Page_Small, heap_metadata_free);
        assert(0);
        return Error_No_Memory;
    }

    heap_metadata_used = slab_alloc(Page_Small, Memory_Writable);
    region_vector_init(heap_metadata_used);
    heap_metadata_free = slab_alloc(Page_Small, Memory_Writable);
    region_vector_init(heap_metadata_free);
    heap_address.virtual = heap;
    region_free(heap_metadata_free, heap_address, 16 * Page_Small);
    memset(heap, (int)Heap_Red_Zone_Fill, Page_Small);
    return Ok;
}

void *kmalloc(size_t bytes) {
    union address address;
    size_t allocation_size = bytes + 2 * Heap_Red_Zone_Size;

    debug_region_vector(heap_metadata_free);
    enum status status = region_alloc(heap_metadata_free,
                                      allocation_size, &address);
    if (status != Ok) {
        return NULL;
    }
    region_free(heap_metadata_used, address, allocation_size);
    unsigned long *redzone_prefix = address.virtual;
    unsigned long *redzone_suffix =
        (unsigned long *)(address.bytes + bytes + Heap_Red_Zone_Size);
    assert(*redzone_prefix == Heap_Red_Zone_Fill &&
           *redzone_suffix == Heap_Red_Zone_Fill);
    return address.bytes + Heap_Red_Zone_Size;
}

void *kcalloc(size_t count, size_t size) {
    void *address = kmalloc(count * size);
    if (address == NULL) {
        return NULL;
    }
    memset(address, 0, count * size);
    return address;
}

void kfree(void *address) {
    union address addr;
    addr.virtual = address;
    size_t size = region_find_size_and_free(heap_metadata_used, addr);
    region_free(heap_metadata_free, addr, size);
}
