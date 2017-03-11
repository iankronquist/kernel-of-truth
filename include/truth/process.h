#pragma once

#include <truth/types.h>
#include <truth/object.h>

enum process_state {
    Process_Running,
    Process_Exited,
};

enum thread_state {
    Thread_Running,
    Thread_Sleeping,
    Thread_Exited,
};


struct thread {
    bool user_space;
    unsigned int id;
    int exit_code;
    enum thread_state state;
    size_t user_stack_size;
    size_t kernel_stack_size;
    uint64_t *user_stack;
    uint64_t *current_stack_pointer;
    uint64_t *kernel_stack;
    struct thread *next;
    struct thread *prev;
    struct process *process;
    struct object obj;
};

struct process {
    unsigned int id;
    int exit_code;
    enum process_state state;
    unsigned int thread_count;
    unsigned int thread_capacity;
    size_t child_count;
    size_t child_capacity;
    struct process *children;
    struct process *parent;
    struct thread **threads;
    struct page_table *page_table;
    struct region_vector *address_space;
    struct object obj;
};

enum status processes_init(void);
void processes_fini(void);

struct thread *thread_spawn(struct process *process);
int thread_exit(struct thread *thread, int exit_code);
struct process *process_spawn(void);
void process_exit(struct process *process, int exit_code);
void thread_switch(struct thread *old_thread, struct thread *new_thread);

/*
Spawn process:
1. Get pid.
2. Alloc address space.
3. Alloc page table.
4. Set process state.
5. Spawn thread 0.

Spawn thread:
1. Get tid.
2. Alloc kernel stack.
3. Alloc user stack.
4. Set up user stack.
5. Set thread state.
6. Schedule thread.

unsigned int process_get_next_pid(void);
unsigned int process_get_next_tid(struct process *proc);
*/
