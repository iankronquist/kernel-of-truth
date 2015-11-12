#include <libk/kmem.h>

void *heap_end = KHEAP_PHYS_ROOT + PAGE_SIZE;

void *kmalloc(size_t bytes) {
    struct kheap_metadata *cur = root;
    // Find a free block.
    while (cur->is_free == false && cur->size < bytes) {
        // If we hit the end of the heap extend it.
        if (cur->next == KHEAP_END_SENTINEL) {
            kheap_extend(bytes);
        }
        cur = cur->next;
    }
    // Now that we have a free block in hand, get the address of the memory,
    // which is right after the metadata.
    void *actual_memory = cur + sizeof(struct kheap_metadata);
    // If our current block is bigger than we need, partition it.
    if (bytes + sizeof(struct kheap_metadata) > cur->size) {
        struct kheap_metadata *new = actual_memory + bytes;
        new->next = cur->next;
        cur->next = new;
        new->is_free = true;
    }
    cur->is_free = false;
    return actual_memory;
}

// Extend heap by page sized increments
void kheap_extend(size_t bytes) {
    size_t new_bytes = (bytes/PAGE_SIZE + 1);
    if (heap_end + new_bytes > KHEAP_PHYS_END) {
        kputs("Out of kernel heap memory");
        kabort();
    }
    heap_end += new_bytes;
}

void kfree(void *mem) {
    // Get the metadata section right before the memory.
    struct kheap_metadata *freeme = mem - sizeof(struct kheap_metadata);
    // Mark the block as free.
    freeme->is_free = true;
    // If the next block exists and is free, merge the two.
    if (freeme->next != KHEAP_END_SENTINEL && freeme->next->is_free) {
        freeme->next = freeme->next->next;
    }
}

// Install the heap
void kheap_install() {
    struct kheap_metadata *root = KHEAP_PHYS_ROOT;
    root->next = NULL;
    root->is_free = true;
    root->size = 0;
    heap_end = KHEAP_PHYS_ROOT + sizeof(struct kheap_metadata);
}

// Crude approach to defragmenting the heap -- if two consecutive blocks are
// free, coalesce them and repeat.
void kheap_defragment() {
    struct kheap_metadata *cur = root;
    while (cur->next != KHEAP_END_SENTINEL) {
        if (cur->is_free && cur->next->is_free) {
            cur->next = cur->next->next;
            continue;
        }
        cur = cur->next;
    }
}
