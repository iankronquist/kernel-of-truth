#include <libk/kmem.h>
#include <libk/process_table.h>


struct process_table *init_process_table() {
    struct process_table *table = kmalloc(sizeof(struct process_table));
    table->head = PROCESS_TABLE_END_SENTINEL;
    return table;
}

int remove_pid(struct process_table *table, true_pid_t pid) {
    if (table->head == PROCESS_TABLE_END_SENTINEL) {
        return -1;
    }
    if (table->head->id == pid) {
        struct process *free_me = table->head;
        table->head = table->head->next;
        kfree(free_me);
        return 0;
    }
    struct process *cur = table->head;
    do {
        if (cur->next == PROCESS_TABLE_END_SENTINEL) {
            return -1;
        }
        cur = cur->next;
    } while (cur->next->id != pid);
    struct process *free_me = cur->next;
    cur = cur->next->next;
    kfree(free_me);

    return 0;
}

void insert_pid(struct process_table *table, struct process *proc) {
    proc->next = table->head;
    table->head = proc;
}

bool contains_pid(struct process_table *table, true_pid_t id) {
    struct process *cur = table->head;
    while (cur != PROCESS_TABLE_END_SENTINEL) {
        if (cur->id == id) {
            return true;
        }
    }
    return false;
}
