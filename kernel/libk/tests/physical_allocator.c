#include <stdlib.h>
#include <inttypes.h>

#include "../../libk/physical_allocator.c"
#include <tests/tests.h>


void reset_bitmap();

// TODO: Finish this test
void test_rebuild_cache() {
    /*
    reset_bitmap();
    page_frame_t cache_copy[PAGE_FRAME_CACHE_SIZE];
    memcpy(cache_copy, frame_cache,
            PAGE_FRAME_CACHE_SIZE*sizeof(page_frame_t));
            */
}

void test_use_frame() {
    reset_bitmap();

    use_frame(0);
    EXPECT_EQ(page_frame_map[0], 1);
    use_frame(1);
    EXPECT_EQ(page_frame_map[0], 1);
    use_frame(PAGE_SIZE-1);
    EXPECT_EQ(page_frame_map[0], 1);

    EXPECT_EQ(frame_count, PAGE_FRAME_CACHE_SIZE);

    use_frame(1*PAGE_SIZE);
    EXPECT_EQ(page_frame_map[0], 3);
    use_frame(2*PAGE_SIZE);
    EXPECT_EQ(page_frame_map[0], 7);
    use_frame(3*PAGE_SIZE);
    EXPECT_EQ(page_frame_map[0], 15);
    use_frame(4*PAGE_SIZE);
    EXPECT_EQ(page_frame_map[0], 31);
    use_frame(5*PAGE_SIZE);
    EXPECT_EQ(page_frame_map[0], 63);
    use_frame(6*PAGE_SIZE);
    EXPECT_EQ(page_frame_map[0], 127);
    use_frame(7*PAGE_SIZE);
    EXPECT_EQ(page_frame_map[0], 255);
    use_frame(8*PAGE_SIZE);
    EXPECT_EQ(page_frame_map[1], 1);
    EXPECT_EQ(page_frame_map[0], 255);
    EXPECT_EQ(frame_count, PAGE_FRAME_CACHE_SIZE);

    use_frame(817*PAGE_SIZE);
    EXPECT_EQ(page_frame_map[102], 2);
    EXPECT_EQ(page_frame_map[0], 255);
    EXPECT_SLICE_EMPTY(page_frame_map, 2, 102);
    EXPECT_EQ(frame_count, PAGE_FRAME_CACHE_SIZE);

    use_frame(82*PAGE_SIZE);
    EXPECT_SLICE_EMPTY(page_frame_map, 2, 10);
    EXPECT_EQ(page_frame_map[10], 4);
    EXPECT_SLICE_EMPTY(page_frame_map, 11, 102);
    EXPECT_EQ(frame_count, PAGE_FRAME_CACHE_SIZE);

    use_frame(478007);
    EXPECT_EQ(page_frame_map[14], 16);
    EXPECT_EQ(frame_count, PAGE_FRAME_CACHE_SIZE);
}

void reset_bitmap() {
    memset(page_frame_map, 0, PAGE_FRAME_MAP_SIZE);
    memset(frame_cache, 0, PAGE_FRAME_CACHE_SIZE);
    frame_count = PAGE_FRAME_CACHE_SIZE;
}

void print_map() {
    for (size_t i = 0; i < PAGE_FRAME_MAP_SIZE; ++i) {
        printf("%h" PRIu8, page_frame_map[i]);
    }
    printf("\n");
}

void print_cache() {
    for (size_t i = 0; i < PAGE_FRAME_CACHE_SIZE; ++i) {
        printf("%h, " PRIu32, frame_cache[i]);
    }
    printf("\n");
}

void test_free_frame() {
    reset_bitmap();
    free_frame(0);
    EXPECT_EQ(page_frame_map[102], 0);
    // Test idempotence
    free_frame(0);
    EXPECT_EQ(page_frame_map[102], 0);

    page_frame_map[1023] = 0xf7;
    free_frame(8187*PAGE_SIZE);
    EXPECT_EQ(page_frame_map[1023], 0xf7);
    free_frame(8188*PAGE_SIZE);
    EXPECT_EQ(page_frame_map[1023], 0xe7);
}

void test_alloc_frame() {
    reset_bitmap();
    EXPECT_EQ(frame_count, PAGE_FRAME_CACHE_SIZE);
    page_frame_t first = alloc_frame();
    printf("%u\n", first);
    EXPECT_EQ(first % PAGE_SIZE, 0);
    EXPECT_EQ(page_frame_map[0], 1);
    EXPECT_EQ(frame_count, 1);

    page_frame_t second = alloc_frame();
    printf("%u\n", second);
    EXPECT_EQ(second % PAGE_SIZE, 0);
    EXPECT_EQ(page_frame_map[0], 3);
    EXPECT_EQ(frame_count, 2);

    page_frame_t third = alloc_frame();
    printf("%u\n", third);
    EXPECT_EQ(third % PAGE_SIZE, 0);
    EXPECT_EQ(page_frame_map[0], 7);
    EXPECT_EQ(frame_count, 3);
}

int main() {
    test_alloc_frame();
    test_free_frame();
    test_use_frame();
    test_rebuild_cache();
    return RETURN_VALUE;
}
