#include <libk/kmem.h>
#include <libk/process_table.h>


struct process_table *init_process_table() {
    struct process_table *table = kmalloc(sizeof(struct process_table));
    table->head = PROCESS_TABLE_END_SENTINEL;
    table->tail = PROCESS_TABLE_END_SENTINEL;
    return table;
}

int remove_pid(struct process_table *table, true_pid_t pid) {
    if (table->head == PROCESS_TABLE_END_SENTINEL) {
        return -1;
    }
    if (table->head->id == pid) {
        struct process *free_me = table->head;
        table->head = table->head->next;
        if (table->head == PROCESS_TABLE_END_SENTINEL) {
            table->tail = PROCESS_TABLE_END_SENTINEL;
        }
        kfree(free_me);
        return 0;
    }
    struct process *cur = table->head;
    while (cur->next->id != pid) {
        cur = cur->next;
        if (cur->next == PROCESS_TABLE_END_SENTINEL) {
            return -1;
        }
    }
    struct process *free_me = cur->next;
    cur->next = cur->next->next;
    if (free_me->next == PROCESS_TABLE_END_SENTINEL) {
        table->tail = cur;
    }
    kfree(free_me);

    return 0;
}

void insert_pid(struct process_table *table, struct process *proc) {
    proc->next = table->head;
    table->head = proc;
    if (table->tail == PROCESS_TABLE_END_SENTINEL) {
        table->tail = proc;
    }
}

bool contains_pid(struct process_table *table, true_pid_t id) {
    struct process *cur = table->head;
    while (cur != PROCESS_TABLE_END_SENTINEL) {
        if (cur->id == id) {
            return true;
        }
        cur = cur->next;
    }
    return false;
}
