#include <arch/x64/paging.h>

#include <truth/hashtable.h>
#include <truth/heap.h>
#include <truth/lock.h>
#include <truth/panic.h>
#include <truth/process.h>
#include <truth/region_vector.h>
#include <truth/scheduler.h>
#include <truth/slab.h>
#include <truth/types.h>

#define Memory_Bootstrap_Stack_Size (16 * KB)

#define Thread_Default_User_Stack_Size (16 * KB)
#define Thread_Default_Kernel_Stack_Size (8 * KB)
#define Thread_Pool_Default_Size 10
#define Process_Pool_Default_Size 100

struct lock Process_Pool_Lock;
struct hashtable *Process_Pool;



extern uint64_t _init_stack_top;
extern void _thread_switch(uint64_t *new_stack, uint64_t **old_stack);


void thread_switch(struct thread *old_thread,
                          struct thread *new_thread) {
    _thread_switch(old_thread->current_stack_pointer,
            &new_thread->current_stack_pointer);
}

static enum status process_add_pool(struct process *proc) {
    enum status status;
    static unsigned int next_pid = 0;
    unsigned int start_pid = next_pid;

    assert(proc != NULL);

    lock_acquire_writer(&Process_Pool_Lock);

    while (hashtable_get(Process_Pool, (union hashtable_key)next_pid) != NULL)
    {
        if (next_pid == start_pid) {
            status = Error_Count;
            goto out;
        }
        next_pid++;
    }
    proc->id = next_pid;
    status = hashtable_put(Process_Pool, (union hashtable_key)proc->id,
                           proc);

out:
    lock_release_writer(&Process_Pool_Lock);
    return status;
}

static enum status process_add_thread(struct process *proc,
                                      struct thread *thread) {
    assert(proc != NULL);
    assert(thread != NULL);
    for (size_t i = 0; i < proc->thread_capacity; ++i) {

        if ((void *)proc->threads[i] == NULL) {
            proc->threads[i] = thread;
            thread->id = i;
            return Ok;
        }

    }
    thread->id = proc->thread_capacity;
    proc->thread_capacity += 10;
    proc->threads = krealloc(proc->threads, proc->thread_capacity);
    if (proc->threads == NULL) {
        return Error_No_Memory;
    }
    proc->threads[thread->id] = thread;
    object_retain(&proc->obj);
    return Ok;
}

void thread_fini(struct thread *thread, int exit_code) {
    thread->exit_code = exit_code;
    thread->state = Thread_Exited;
    scheduler_remove_thread(thread);
}

struct region_vector *process_create_address_space(void) {
    return region_vector_new((union address)Lower_Half_Start, Lower_Half_Size);
}

static void process_free(struct object *obj) {
    struct process *process = container_of(obj, struct process *, obj);
    assert(process->thread_count == 0);
    lock_acquire_writer(&Process_Pool_Lock);
    enum status unused(_) = hashtable_remove(Process_Pool,
                                             (union hashtable_key)process->id);
    lock_release_writer(&Process_Pool_Lock);
    for (size_t i = 0; i < process->thread_count; ++i) {
        kfree(process->threads[i]);
    }
    region_vector_fini(process->address_space);
    kfree(process->threads);
    kfree(process);
}

static void thread_free(struct object *obj) {
    struct thread *thread = container_of(obj, struct thread *, obj);
    assert(thread->next == NULL);
    assert(thread->prev == NULL);
    assert(thread->state = Thread_Exited);
    slab_free(thread->user_stack_size, thread->user_stack);
    slab_free(thread->kernel_stack_size, thread->kernel_stack);
    object_release(&thread->process->obj);
    kfree(thread);
}

void bootstrap_thread_init(struct process *proc) {
    assert(proc != NULL);
    struct thread *thread = proc->threads[0];
    thread->user_space = false;

    slab_free(thread->kernel_stack_size, thread->kernel_stack);
    thread->kernel_stack_size = Memory_Bootstrap_Stack_Size;
    thread->kernel_stack = &_init_stack_top;
}

static void thread_user_stack_init(struct thread *thread, void *entry_point) {
    assert(thread != NULL);
    assert(thread->user_stack != NULL);
    size_t last_stack_index = thread->user_stack_size / sizeof(uint64_t) - 1;
    thread->user_stack[last_stack_index] = (uintptr_t)entry_point;
    // 16 GPRs, 1 rflags
    thread->current_stack_pointer = (void *)last_stack_index -
        (17 * sizeof(uint64_t));
}

struct thread *thread_init(struct process *proc, void *entry_point,
                           bool user_space) {
    enum status status;


    struct thread *thread = kmalloc(sizeof(struct thread));
    if (thread == NULL) {
        return NULL;
    }

    thread->kernel_stack_size = Thread_Default_Kernel_Stack_Size;
    thread->kernel_stack = slab_alloc(thread->kernel_stack_size, Memory_Writable);
    if (thread->kernel_stack == NULL) {
        kfree(thread);
        return NULL;
    }

    if (user_space) {
        thread->user_stack_size = Thread_Default_User_Stack_Size;
        thread->user_stack = slab_alloc(thread->user_stack_size, Memory_Writable |
                                        Memory_User_Access);
        if (thread->user_stack == NULL) {
            slab_free(thread->kernel_stack_size, thread->kernel_stack);
            kfree(thread);
            return NULL;
        }
        thread->current_stack_pointer = thread->user_stack + thread->user_stack_size;
    } else {
        thread->user_stack_size = 0;
        thread->user_stack = NULL;
        thread->current_stack_pointer = NULL;
    }

    status = process_add_thread(proc, thread);
    if (status != Ok) {
        slab_free(thread->user_stack_size, thread->user_stack);
        slab_free(thread->kernel_stack_size, thread->kernel_stack);
        kfree(thread);
    }

    thread->exit_code = 0;
    thread->state = Thread_Running;
    thread->next = NULL;
    thread->prev = NULL;
    thread->process = proc;

    thread_user_stack_init(thread, entry_point);

    object_clear(&thread->obj, thread_free);
    scheduler_add_thread(thread);

    return thread;
}

struct process *process_init(void *entry_point) {
    enum status status;
    struct region_vector *vect = NULL;
    struct process *proc = NULL;
    struct thread *thread = NULL;
    struct thread **thread_pool = NULL;
    struct page_table *page_table = NULL;

    proc = kmalloc(sizeof(struct process));
    if (proc == NULL) {
        status = Error_No_Memory;
        goto err;
    }

    thread_pool = kcalloc(Thread_Pool_Default_Size, sizeof(struct thread *));
    if (thread_pool == NULL) {
        status = Error_No_Memory;
        goto err;
    }
    proc->threads = thread_pool;

    thread = thread_init(proc, entry_point, true);
    if (thread == NULL) {
        status = Error_No_Memory;
        goto err;
    }

    vect = process_create_address_space();
    if (vect == NULL) {
        status = Error_No_Memory;
        goto err;
    }

    page_table = page_table_init();
    if (page_table == NULL) {
        status = Error_No_Memory;
        goto err;
    }

	log(Log_Error, "~~ 7\n");
    status = process_add_pool(proc);
    if (status != Ok) {
        goto err;
    }

	log(Log_Error, "~~ 8\n");
    proc->exit_code = 0;
    proc->state = Process_Running;
    proc->thread_count = 1;
    proc->thread_capacity = Thread_Pool_Default_Size;
    proc->child_count = 0;
    proc->child_capacity = 0;
    proc->children = NULL;
    proc->parent = NULL;
    proc->page_table = page_table;
    proc->address_space = vect;
    object_clear(&proc->obj, process_free);


    object_retain(&proc->obj);

    return proc;

err:
    kfree(page_table);
    region_vector_fini(vect);
    kfree(thread_pool);
    kfree(thread);
    kfree(proc);
    return NULL;
}

static size_t process_hash_pid(union hashtable_key key) {
    return key.integer;
}

static enum partial process_comp_pid(union hashtable_key a,
                                     union hashtable_key b) {
    struct process *proc_a = a.ptr;
    struct process *proc_b = b.ptr;
    if (proc_a == proc_b) {
        return Partial_Equal;
    } else {
        return Partial_Not_Equal;
    }
}

enum status processes_init(void) {
    Process_Pool = hashtable_init(Process_Pool_Default_Size, process_hash_pid,
                                  process_comp_pid);
    if (Process_Pool == NULL) {
        return Error_No_Memory;
    }
    struct process *init = process_init(NULL);
    bootstrap_thread_init(init);
    object_release(&init->obj);
    return Ok;
}

void process_fini(struct process *proc, int exit_code) {

    proc->exit_code = exit_code;

    region_vector_fini(proc->address_space);
    proc->address_space = NULL;
    page_table_fini(proc->page_table);
    proc->page_table = NULL;

    for (size_t i = 0; i < proc->thread_count; ++i) {
        thread_fini(proc->threads[i], exit_code);
    }
}
