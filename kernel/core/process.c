#include <arch/x64/paging.h>
#include <truth/hashtable.h>
#include <truth/heap.h>
#include <truth/lock.h>
#include <truth/memory.h>
#include <truth/panic.h>
#include <truth/process.h>
#include <truth/region_vector.h>
#include <truth/scheduler.h>
#include <truth/slab.h>
#include <truth/string.h>
#include <truth/types.h>


struct lock Process_Pool_Lock = Lock_Clear;
struct hashtable *Process_Pool;

#define Thread_Pool_Default_Size 10
#define Process_Pool_Default_Size 100


extern uint64_t _init_stack_top;


static enum status process_add_pool(struct process *proc) {
    enum status status;
    union hashtable_key key;
    static unsigned int next_pid = 0;
    unsigned int start_pid = next_pid;

    assert(proc != NULL);

    lock_acquire_writer(&Process_Pool_Lock);
    key.data = next_pid;

    while (hashtable_get(Process_Pool, key) != NULL)
    {
        next_pid++;
        key.data = next_pid;
        if (next_pid == start_pid) {
            status = Error_Full;
            goto out;
        }
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
    return region_vector_new(Lower_Half_Start, Lower_Half_Size);
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

    slab_free_helper(thread->user_stack_size, thread->user_stack,
                     thread->process->address_space, true);
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


struct thread *thread_init(struct process *proc, void *entry_point,
                           bool user_space) {
    enum status status;


    struct thread *thread = kmalloc(sizeof(struct thread));
    if (thread == NULL) {
        return NULL;
    }
    thread->user_space = user_space;

    thread->kernel_stack_size = Thread_Default_Kernel_Stack_Size;
    thread->kernel_stack = slab_alloc(thread->kernel_stack_size,
                                      Memory_Writable);
    if (thread->kernel_stack == NULL) {
        kfree(thread);
        return NULL;
    }

    if (user_space) {
        thread->user_stack_size = Thread_Default_User_Stack_Size;
        status = thread_user_stack_init(thread, proc, entry_point);
        if (status != Ok) {
            slab_free(thread->kernel_stack_size, thread->kernel_stack);
            kfree(thread);
            return NULL;
        }
    } else {
        thread->user_stack_size = 0;
        thread->user_stack = NULL;
        thread->current_stack_pointer = NULL;
    }

    status = process_add_thread(proc, thread);
    if (status != Ok) {
        slab_free_helper(thread->user_stack_size, thread->user_stack,
                         proc->address_space, true);
        slab_free(thread->kernel_stack_size, thread->kernel_stack);
        kfree(thread);
    }

    thread->exit_code = 0;
    thread->state = Thread_Running;
    thread->next = NULL;
    thread->prev = NULL;
    thread->process = proc;

    object_clear(&thread->obj, thread_free);

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
        goto err;
    }

    thread_pool = kcalloc(Thread_Pool_Default_Size, sizeof(struct thread *));
    if (thread_pool == NULL) {
        goto err;
    }
    proc->threads = thread_pool;

    vect = process_create_address_space();
    if (vect == NULL) {
        goto err;
    }
    proc->address_space = vect;

    page_table = page_table_init();
    if (page_table == NULL) {
        goto err;
    }
    proc->page_table = page_table;

    thread = thread_init(proc, entry_point, true);
    if (thread == NULL) {
        goto err;
    }

    status = process_add_pool(proc);
    if (status != Ok) {
        goto err;
    }

    proc->exit_code = 0;
    proc->state = Process_Running;
    proc->thread_count = 1;
    proc->thread_capacity = Thread_Pool_Default_Size;
    proc->child_count = 0;
    proc->child_capacity = 0;
    proc->children = NULL;
    proc->parent = NULL;
    object_clear(&proc->obj, process_free);
    object_retain(&proc->obj);

    scheduler_add_thread(thread);

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
    const struct process *proc_a = a.ptr;
    const struct process *proc_b = b.ptr;
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
