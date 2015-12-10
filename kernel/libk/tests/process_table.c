#include <stdlib.h>
#include <tests/tests.h>
#include <libk/kmem.h>
#include <libk/process_table.h>

int main() {
    struct process_table *t = init_process_table();
    struct process *p0 = kmalloc(sizeof(struct process));
    p0->id = 0;
    struct process *p1 = kmalloc(sizeof(struct process));
    p1->id = 1;
    struct process *p2 = kmalloc(sizeof(struct process));
    p2->id = 2;
    struct process *p3 = kmalloc(sizeof(struct process));
    p3->id = 3;
    int ret;

    // Nothing is in the table yet
    ret = remove_pid(t, 0);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(t->head, PROCESS_TABLE_END_SENTINEL);

    insert_pid(t, p0);
    EXPECT_EQ(t->head, p0);
    EXPECT_EQ(p0->next, PROCESS_TABLE_END_SENTINEL);

    insert_pid(t, p1);
    EXPECT_EQ(t->head, p1);
    EXPECT_EQ(p1->next, p0);
    EXPECT_EQ(p0->next, PROCESS_TABLE_END_SENTINEL);

    insert_pid(t, p2);
    EXPECT_EQ(t->head, p2);
    EXPECT_EQ(p2->next, p1);
    EXPECT_EQ(p1->next, p0);
    EXPECT_EQ(p0->next, PROCESS_TABLE_END_SENTINEL);

    EXPECT_EQ(contains_pid(t, p1->id), true);

    ret = remove_pid(t, p1->id);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(t->head, p2);
    EXPECT_EQ(p2->next, p0);
    EXPECT_EQ(p0->next, PROCESS_TABLE_END_SENTINEL);

    EXPECT_EQ(contains_pid(t, p1->id), false);

    insert_pid(t, p3);
    EXPECT_EQ(t->head, p3);

    ret = remove_pid(t, p3->id);
    EXPECT_EQ(contains_pid(t, p3->id), false);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(t->head, p2);
    EXPECT_EQ(p2->next, p0);
    EXPECT_EQ(p0->next, PROCESS_TABLE_END_SENTINEL);

    // Already removed. Should fail.
    ret = remove_pid(t, p3->id);
    EXPECT_EQ(ret, -1);
    // Nothing changed
    EXPECT_EQ(t->head, p2);
    EXPECT_EQ(p2->next, p0);
    EXPECT_EQ(p0->next, PROCESS_TABLE_END_SENTINEL);





    return RETURN_VALUE;
}
