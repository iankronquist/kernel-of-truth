#include <stdlib.h>

#include <tests/tests.h>
#include <libk/kmem.h>

int main() {
    char *playground = malloc(PAGE_SIZE * 5);
    kheap_install((void*)playground);
    struct kheap_metadata *root = (struct kheap_metadata*)playground;
    EXPECT_EQ(root->next, KHEAP_END_SENTINEL);
    EXPECT_EQ(root->is_free, true);
    EXPECT_EQ(root->size, 0);

    void *first = kmalloc(100);
    EXPECT_EQ(root, first + sizeof(struct kheap_metadata));
    EXPECT_EQ(root->next, KHEAP_END_SENTINEL);
    EXPECT_EQ(root->is_free, false);
    EXPECT_EQ(root->size, 100);

    void *second = kmalloc(200);
    struct kheap_metadata *second_metadata = second -
        sizeof(struct kheap_metadata);
    // root's next element is second, but nothing else changes about root
    EXPECT_EQ(root->next, second);
    EXPECT_EQ(root->is_free, false);
    EXPECT_EQ(root->size, 100);

    // second is not free. The next block is the end
    EXPECT_EQ(second_metadata->next, KHEAP_END_SENTINEL);
    EXPECT_EQ(second_metadata->is_free, false);
    EXPECT_EQ(second_metadata->size, 200);

    void *third = kmalloc(300);
    struct kheap_metadata *third_metadata = third -
        sizeof(struct kheap_metadata);
    // nothing else changes about root
    EXPECT_EQ(root->next, second);
    EXPECT_EQ(root->is_free, false);
    EXPECT_EQ(root->size, 100);

    // second's next element is third, but nothing else changes
    EXPECT_EQ(second_metadata->next, third);
    EXPECT_EQ(second_metadata->is_free, false);
    EXPECT_EQ(second_metadata->size, 200);

    // third is not free. The next block is the end
    EXPECT_EQ(third_metadata->next, KHEAP_END_SENTINEL);
    EXPECT_EQ(third_metadata->is_free, false);
    EXPECT_EQ(third_metadata->size, 300);



    kfree(second);

    // nothing changes
    EXPECT_EQ(root->next, second);
    EXPECT_EQ(root->is_free, false);
    EXPECT_EQ(root->size, 100);

    // second is free but nothing else changes
    EXPECT_EQ(second_metadata->next, third);
    EXPECT_EQ(second_metadata->is_free, true);
    EXPECT_EQ(second_metadata->size, 200);

    // nothing changes for third
    EXPECT_EQ(third_metadata->next, KHEAP_END_SENTINEL);
    EXPECT_EQ(third_metadata->is_free, false);
    EXPECT_EQ(third_metadata->size, 300);


    kfree(root);

    // root is free but nothing else changes
    EXPECT_EQ(root->next, second);
    EXPECT_EQ(root->is_free, true);
    EXPECT_EQ(root->size, 100);

    // nothing changes for second
    EXPECT_EQ(second_metadata->next, third);
    EXPECT_EQ(second_metadata->is_free, true);
    EXPECT_EQ(second_metadata->size, 200);

    // nothing changes for third
    EXPECT_EQ(third_metadata->next, KHEAP_END_SENTINEL);
    EXPECT_EQ(third_metadata->is_free, false);
    EXPECT_EQ(third_metadata->size, 300);

    // Allocate a new element the size of second
    void *fourth = kmalloc(200);
    EXPECT_EQ(second, fourth);

    kfree(third);
    EXPECT_EQ(third_metadata->next, KHEAP_END_SENTINEL);
    EXPECT_EQ(third_metadata->is_free, true);
    EXPECT_EQ(third_metadata->size, 300);

    // Allocate a new element bigger than second but smaller than third
    void *fifth = kmalloc(250);
    EXPECT_EQ(fifth, third);
    EXPECT_EQ(third_metadata->is_free, false);
    EXPECT_EQ(third_metadata->size, 250);
    EXPECT_EQ(third_metadata->next, third + 50);

    struct kheap_metadata *sixth_metadata = third + 50;
    EXPECT_EQ(sixth_metadata->is_free, true);
    EXPECT_EQ(sixth_metadata->size, 50 - sizeof(struct kheap_metadata));
    EXPECT_EQ(sixth_metadata->next, KHEAP_END_SENTINEL);

    free(playground);
}
