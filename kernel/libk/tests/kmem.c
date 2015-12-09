#include <stdlib.h>
#include <assert.h>

#include <tests/tests.h>
#include <libk/kmem.h>
#include "../kmem.c"

// Useful for debugging
void kprint_heap() {
    struct kheap_metadata *cur = root;
    while (cur != KHEAP_END_SENTINEL) {
        printf("location: %p\n", cur);
        printf("is free?: %s\n", cur->is_free ? "yes": "no");
        printf("size: %zu\n", cur->size);
        printf("next: %p\n", cur->next);
        cur = cur->next;
    }
}

int main() {
    size_t playground_size = PAGE_SIZE * 5;
    char *playground = malloc(PAGE_SIZE * 5);
    dependencies_suck = (uintptr_t)playground;
    memset(playground, 0, playground_size);

    puts("Initializing heap");
    kheap_install((void*)playground, playground_size);
    struct kheap_metadata *root = (struct kheap_metadata*)playground;
    EXPECT_EQ(root->next, KHEAP_END_SENTINEL);
    EXPECT_EQ(root->is_free, true);
    EXPECT_EQ(root->size, PAGE_SIZE * 5);

    puts("Creating first allocation");
    void *first = kmalloc(100);
    struct kheap_metadata *first_md = root + 100 + sizeof(struct kheap_metadata);
    EXPECT_EQ(first, root + 1);
    EXPECT_EQ(root->next, root + sizeof(struct kheap_metadata) + 100);
    EXPECT_EQ(root->is_free, false);
    EXPECT_EQ(root->size, 100);

    EXPECT_EQ(first_md->next, KHEAP_END_SENTINEL);
    EXPECT_EQ(first_md->size, playground_size - 100 -
            sizeof(struct kheap_metadata));
    EXPECT_EQ(first_md->is_free, true);

    puts("Creating second allocation");
    void *second = kmalloc(200);
    struct kheap_metadata *second_md = first_md + 200 +
        sizeof(struct kheap_metadata);
    EXPECT_EQ(second, first_md + 1);
    EXPECT_EQ(first, root + 1);
    EXPECT_EQ(root->next, first_md);
    EXPECT_EQ(root->is_free, false);
    EXPECT_EQ(root->size, 100);

    EXPECT_EQ(first_md->next, second_md);
    EXPECT_EQ(first_md->is_free, false);
    EXPECT_EQ(first_md->size, 200);

    EXPECT_EQ(second_md->next, KHEAP_END_SENTINEL);
    EXPECT_EQ(second_md->is_free, true);
    // We're using 2 kheap_metadata structures which have a total of 300 bytes
    EXPECT_EQ(second_md->size, playground_size - 300 -
            2 * sizeof(struct kheap_metadata));

    puts("Creating third allocation");
    void *third = kmalloc(300);
    struct kheap_metadata *third_md = second_md + 300 +
        sizeof(struct kheap_metadata);
    EXPECT_EQ(third, second_md + 1);
    EXPECT_EQ(root->next, first_md);
    EXPECT_EQ(root->is_free, false);
    EXPECT_EQ(root->size, 100);

    EXPECT_EQ(first_md->next, second_md);
    EXPECT_EQ(first_md->is_free, false);
    EXPECT_EQ(first_md->size, 200);

    EXPECT_EQ(second_md->next, third_md);
    EXPECT_EQ(second_md->is_free, false);
    // We're using 2 kheap_metadata structures which have a total of 300 bytes
    EXPECT_EQ(second_md->size, 300);

    EXPECT_EQ(third_md->next, KHEAP_END_SENTINEL);
    EXPECT_EQ(third_md->is_free, true);
    // We're using 3 kheap_metadata structures which have a total of 600 bytes
    EXPECT_EQ(third_md->size, playground_size - 600 -
            3 * sizeof(struct kheap_metadata));


    puts("Freeing second allocation");
    kfree(second);

    EXPECT_EQ(second, first_md + 1);

    EXPECT_EQ(root->next, first_md);
    EXPECT_EQ(root->is_free, false);
    EXPECT_EQ(root->size, 100);

    EXPECT_EQ(first_md->next, second_md);
    EXPECT_EQ(first_md->is_free, true);
    EXPECT_EQ(first_md->size, 200);

    EXPECT_EQ(second_md->next, third_md);
    EXPECT_EQ(second_md->is_free, false);
    // We're using 2 kheap_metadata structures which have a total of 300 bytes
    EXPECT_EQ(second_md->size, 300);

    EXPECT_EQ(third_md->next, KHEAP_END_SENTINEL);
    EXPECT_EQ(third_md->is_free, true);
    // We're using 3 kheap_metadata structures which have a total of 600 bytes
    EXPECT_EQ(third_md->size, playground_size - 600 -
            3 * sizeof(struct kheap_metadata));

    kfree(first);

    // root is free but nothing else changes
    EXPECT_EQ(root->next, second_md);
    EXPECT_EQ(root->is_free, true);
    EXPECT_EQ(root->size, 324);


    // nothing changes for second
    EXPECT_EQ(second_md->next, third_md);
    EXPECT_EQ(second_md->is_free, false);
    EXPECT_EQ(second_md->size, 300);

    // nothing changes for third
    EXPECT_EQ(third_md->next, KHEAP_END_SENTINEL);
    EXPECT_EQ(third_md->is_free, true);
    EXPECT_EQ(third_md->size, playground_size - 600 -
            3 * sizeof(struct kheap_metadata));

    // Free first again -- nothing should change
    kfree(first);

    EXPECT_EQ(root->next, second_md);
    EXPECT_EQ(root->is_free, true);
    EXPECT_EQ(root->size, 324);

    // Allocate a new element the size of the first block
    void *fourth = kmalloc(300);
    EXPECT_EQ(first, fourth);
    EXPECT_EQ(root->is_free, false);

    // Free all memory
    kfree(third);
    kfree(fourth);
    EXPECT_EQ(root->next, KHEAP_END_SENTINEL);
    EXPECT_EQ(root->is_free, true);
    EXPECT_EQ(root->size, PAGE_SIZE * 5);

    free(playground);
    return RETURN_VALUE;
}
