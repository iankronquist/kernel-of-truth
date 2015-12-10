#include <libk/kmem.h>

void *heap_end = NULL;

void *krealloc(void *ptr, size_t bytes) {
    struct kheap_metadata *ptr_md = ptr - sizeof(struct kheap_metadata);
    // If we are shrinking the block
    if (ptr_md->size >= bytes) {
        // If the new allocation is slightly smaller than the current one, but
        // not small enough to bother with, return the old pointer.
        if (ptr_md->size <= bytes + KHEAP_BLOCK_SLOP) {
            return ptr;
        } else { // However, if it is big enough to partition, split it and
                 // create a new block.
            struct kheap_metadata *new = ptr + bytes;
            new->size = ptr_md->size - bytes - sizeof(struct kheap_metadata);
            new->next = ptr_md->next;
            ptr_md->next = new;
            ptr_md->size = bytes;
            return ptr;
        }
    }
    // We're growing the partition. Walk through the following blocks. If there is a chain
    struct kheap_metadata *cur = ptr_md;
    // FIXME: This is kind of gross.
    size_t gobbled_size = 0;
    do {
        gobbled_size += cur->size + sizeof(struct kheap_metadata);
        cur = cur->next;
        // If we're at the end of the heap, just extend it.
        if (cur == KHEAP_END_SENTINEL) {
            size_t new_bytes = kheap_extend(bytes - gobbled_size +
                    sizeof(struct kheap_metadata));
            ptr_md->size = new_bytes;
            ptr_md->next = KHEAP_END_SENTINEL;
            return ptr;
        }
    } while (cur->is_free &&
             gobbled_size - sizeof(struct kheap_metadata) < bytes);

    if (gobbled_size < bytes) {
        ptr_md->size = gobbled_size - sizeof(struct kheap_metadata);
        ptr_md->next = cur->next;
        return ptr;
    } else {
        void *new = kmalloc(bytes);
        memcpy(new, ptr, ptr_md->size);
        ptr_md->is_free = true;
        return new;
    }
}

void *kmalloc(size_t bytes) {
    struct kheap_metadata *cur = root;
    if (bytes == 0) {
        return NULL;
    }
    // Find the first free block
    while (cur != KHEAP_END_SENTINEL && !cur->is_free && cur->size < bytes) {
        cur = cur->next;
    }
    // If there wasn't one, grow the heap
    if (cur == KHEAP_END_SENTINEL) {
        cur = heap_end;
        size_t new_bytes = kheap_extend(bytes + sizeof(struct kheap_metadata));
        cur->size = new_bytes - sizeof(struct kheap_metadata);
        if (new_bytes == 0) {
            return NULL;
        }
    }
    // Potentially partition the free block
    if (cur->size > bytes + sizeof(struct kheap_metadata) +
            KHEAP_BLOCK_SLOP) {
        struct kheap_metadata *new_block = cur +
            sizeof(struct kheap_metadata) + bytes;
        new_block->is_free = true;

        new_block->size = cur->size - sizeof(struct kheap_metadata) -
            bytes;
        cur->size = bytes;

        new_block->next = cur->next;
        cur->next = new_block;
    }
    cur->is_free = false;
    return cur + 1;
}


// Extend heap by page sized increments
int kheap_extend(size_t bytes) {
    size_t new_bytes = (bytes - 1) / (PAGE_SIZE + 1);
    kassert(new_bytes % PAGE_SIZE == 0);
    if (heap_end + new_bytes > KHEAP_PHYS_END) {
        kputs("Out of kernel heap memory");
        kabort();
    }
    heap_end += new_bytes;
    return new_bytes;
}

void kfree(void *mem) {
    if (mem == NULL) return;
    // Get the metadata section right before the memory.
    struct kheap_metadata *freeme = mem - sizeof(struct kheap_metadata);
    // Mark the block as free.
    freeme->is_free = true;
    // If the next block exists and is free, merge the two.
    while (freeme->next != KHEAP_END_SENTINEL && freeme->next->is_free) {
        freeme->size += freeme->next->size + sizeof(struct kheap_metadata);
        freeme->next = freeme->next->next;
    }
}



// Install the heap
void kheap_install(struct kheap_metadata *new_root, size_t initial_heap_size) {
    root = new_root;
    root->next = KHEAP_END_SENTINEL;
    root->is_free = true;
    heap_end = (void*)(root + initial_heap_size);
    root->size = initial_heap_size;
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
