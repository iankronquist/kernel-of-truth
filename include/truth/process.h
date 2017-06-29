#pragma once

#ifdef __C__
#include <truth/types.h>
#include <truth/object.h>

#define Thread_Default_User_Stack_Size (16 * KB)
#define Thread_Default_Kernel_Stack_Size (8 * KB)

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
    struct thread *self;
    uint64_t *current_stack_pointer;
    uint64_t *user_stack;
    uint64_t *kernel_stack;
    bool user_space;
    unsigned int id;
    int exit_code;
    enum thread_state state;
    size_t user_stack_size;
    size_t kernel_stack_size;
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
enum status thread_user_stack_init(struct thread *thread, struct process *proc,
                                   void *entry_point);

struct thread *thread_get_data(void);
void thread_set_data(struct thread *);

#endif // __C__

#define Thread_Self_GS_Offset 0
#define Thread_Current_Stack_GS_Offset 8


