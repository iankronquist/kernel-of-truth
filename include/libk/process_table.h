#ifndef PROCESS_TABLE_H
#define PROCESS_TABLE_H

#include <libk/proc.h>

#define PROCESS_TABLE_END_SENTINEL NULL
#define PROCESS_TABLE_CHUNK_SIZE 32

// FIXME: Gnarly hack. Move to its own file.
typedef uint32_t true_pid_t;

struct process_table {
    struct process *head;
    struct process *tail;
};

struct process_table *init_process_table();

struct process_table *init_process_table();
int remove_pid(struct process_table *table, true_pid_t pid);
void insert_pid(struct process_table *table, struct process *proc);
bool contains_pid(struct process_table *table, true_pid_t id);
void move_head_to_end(struct process_table *table);

#endif