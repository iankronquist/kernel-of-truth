#include <truth/panic.h>
#include <truth/region_vector.h>
#include <truth/slab.h>
#include <truth/string.h>
#include <truth/types.h>

#define Heap_Red_Zone_Size sizeof(unsigned long)
#define Heap_Red_Zone_Fill 0xccccccccccccccccul

#define Heap_Size 16

struct region_vector *heap_metadata_used;
struct region_vector *heap_metadata_free;
byte *heap = NULL;

enum status checked init_heap(void) {
    union region_address heap_address;
    heap_metadata_used = slab_alloc(Page_Small, Memory_Writable, 'heap');
    if (heap_metadata_used == NULL) {
        assert(0);
        return Error_No_Memory;
    }
    heap_metadata_free = slab_alloc(Page_Small, Memory_Writable, 'heap');
    if (heap_metadata_free == NULL) {
        slab_free(heap_metadata_used, 'heap');
        assert(0);
        return Error_No_Memory;
    }
    heap = slab_alloc(Heap_Size, Page_Small, Memory_Writable);
    if (heap == NULL) {
        slab_free(heap_metadata_used, 'heap');
        slab_free(heap_metadata_free, 'heap');
        assert(0);
        return Error_No_Memory;
    }

    heap_metadata_used = slab_alloc(1, Page_Small, Memory_Writable);
    init_region_vector(heap_metadata_used);
    heap_metadata_free = slab_alloc(1, Page_Small, Memory_Writable);
    init_region_vector(heap_metadata_free);
    heap_address.virtual = heap;
    bubble(region_put_by_address(heap_metadata_free, heap_address, Heap_Size * Page_Small, 'free'), "initializing heap");
    memset(heap, (int)Heap_Red_Zone_Fill, Page_Small);
    return Ok;
}

/*
void *malloc(size_t bytes, int tag) {
    union region_address address;
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

void *calloc(size_t count, size_t size, int tag) {
    void *address = kmalloc(count * size);
    if (address == NULL) {
        return NULL;
    }
    memset(address, 0, count * size);
    return address;
}

void free(void *address, int tag) {
    union region_address addr;
    addr.virtual = address;
    size_t size = region_find_size_and_free(heap_metadata_used, addr);
    region_free(heap_metadata_free, addr, size);
}
*/
