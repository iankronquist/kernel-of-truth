#include <tests/tests.h>
#include "../region.c"

#include <stdlib.h>
#include <time.h>

bool in_array(void *item, void **array, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if (array[i] == item) {
            return true;
        }
    }
    return false;
}

size_t total_size(struct region_head *rh) {
    size_t total = 0;
    struct region *cur = rh->list;
    while (cur != NULL) {
        total += cur->size;
        cur = cur->next;
    }
    return total;
}

void test_random_find_region(void) {
    void *root_ptr = (void*)0xdeadb000;
    void *small_stack[100];
    size_t small_sizes[100];
    size_t cursor = 0;
    struct region_head *rh = init_region_list();
    status_t stat = insert_region(root_ptr, 100 * PAGE_SIZE, rh);
    EXPECT_EQ(stat, Ok);
    EXPECT_EQ(total_size(rh), 100 * PAGE_SIZE);

    for (size_t i = 0; i < 100000; ++i) {
        size_t cur_size = total_size(rh);
        if (cursor < 100 && rand() % 4 != 0 && total_size(rh) > 0) {
                size_t max_pages = (cur_size / PAGE_SIZE) > 100 ? 100 : cur_size / PAGE_SIZE;
                size_t random_region_size = ((rand() % max_pages) + 1) * PAGE_SIZE;
                void *addr = find_region(random_region_size, rh);
                if (addr != NULL) {
                    EXPECT_EQ(in_array(addr, (void**)&small_stack, cursor), false);
                    small_stack[cursor] = addr;
                    small_sizes[cursor] = random_region_size;
                    cursor++;
                    EXPECT_LT(total_size(rh), cur_size);
                    //EXPECT_EQ(cur_size, total_size(rh) + random_region_size);
                }
        } else if (cursor > 0) {
            cursor--;
            status_t stat = insert_region(small_stack[cursor], small_sizes[cursor], rh);
            EXPECT_EQ(stat, Ok);
            EXPECT_GT(total_size(rh), cur_size);
            //EXPECT_EQ(total_size(rh) - cur_size, small_sizes[cursor]);
        }
    }
    destroy_free_list(rh);
}

int main() {
    srand(time(NULL));
    test_random_find_region();
    return RETURN_VALUE;
}
