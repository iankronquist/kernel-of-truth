#include <tests/tests.h>
#include "../region.c"

status_t checked insert_region(void *addr, uint64_t size,
        struct region_head *head);
// Return a newly allocated region.
struct region_head *init_region_list(void);
// Destroy a list of regions.
void destroy_free_list(struct region_head *vr);
// Find a free region of the given size.
void *find_region(size_t size, struct region_head *vr);
// Map a region into the current page process' page table.
void map_region(void *vr, size_t pages,  uint16_t perms);
// Unmap a region from the current page process' page table.
void unmap_region(void *vr, size_t pages);

size_t total_size(struct region_head *rh) {
    size_t total = 0;
    struct region *cur = rh->list;
    while (cur != NULL) {
        total += cur->size;
        cur = cur->next;
    }
    return total;
}

void test_insert_region(void) {
    void *root_ptr = (void*)0xdeadb000;
    const size_t num_allocs = 100;
    struct region_head *rh = init_region_list();
    EXPECT_EQ(rh->list, NULL);
    for (size_t i = 0; i < num_allocs; ++i) {
        status_t stat = insert_region(root_ptr + i * PAGE_SIZE, PAGE_SIZE, rh);
        EXPECT_EQ(stat, Ok);
        EXPECT_NEQ(rh->list, NULL);
        EXPECT_EQ(total_size(rh), (i+1) * PAGE_SIZE);
    }
    EXPECT_EQ(total_size(rh), num_allocs*PAGE_SIZE);
    destroy_free_list(rh);
}

void test_find_region(void) {
    void *root_ptr = (void*)0xdeadb000;
    const size_t num_allocs = 100;
    const size_t init_size = num_allocs * PAGE_SIZE;
    struct region_head *rh = init_region_list();
    EXPECT_EQ(rh->list, NULL);
    status_t stat = insert_region(root_ptr, init_size, rh);
    EXPECT_NEQ(rh->list, NULL);
    EXPECT_EQ(stat, Ok);
    EXPECT_EQ(total_size(rh), init_size);
    for (size_t i = 0; i < num_allocs; ++i) {
        void *ptr = find_region(PAGE_SIZE, rh);
        EXPECT_EQ((void*)PAGE_ALIGN(ptr), ptr);
        EXPECT_GEQ(ptr, root_ptr);
        EXPECT_EQ(total_size(rh), init_size - (i+1) * PAGE_SIZE);
    }
    EXPECT_EQ(rh->list, NULL);
    destroy_free_list(rh);
}

int main() {
    test_insert_region();
    test_find_region();
    return RETURN_VALUE;
}
