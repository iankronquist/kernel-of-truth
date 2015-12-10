#include <stdlib.h>
#include <tests/tests.h>
#include <libk/kmem.h>
#include <libk/process_table.h>

int main() {
    struct process_table *t = init_process_table();
    struct process *p0 = kmalloc(sizeof(struct process));
    struct process *p1 = kmalloc(sizeof(struct process));
    struct process *p2 = kmalloc(sizeof(struct process));

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

    remove_pid(t, p1->id);
    EXPECT_EQ(t->head, p2);
    EXPECT_EQ(p1->next, p1);
    EXPECT_EQ(p0->next, PROCESS_TABLE_END_SENTINEL);

    EXPECT_EQ(contains_pid(t, p1->id), false);

    return RETURN_VALUE;
}
