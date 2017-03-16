#include <truth/lock.h>
#include <truth/scheduler.h>
#include <truth/panic.h>

static struct lock Current_Thread_Lock = Lock_Clear;
static struct thread *Current_Thread = NULL;


enum status scheduler_init(struct process *init) {
    assert(init != NULL);
    assert(init->thread_count > 0);
    Current_Thread = init->threads[0];
    Current_Thread->next = Current_Thread;
    Current_Thread->prev = Current_Thread;
    object_retain(&Current_Thread->obj);
    return Ok;
}


void scheduler_fini(void) {
    struct thread *old;
    lock_acquire_writer(&Current_Thread_Lock);
    while (Current_Thread != Current_Thread->next) {
        old = Current_Thread;
        Current_Thread = Current_Thread->next;
        scheduler_remove_thread(old);
    }
    scheduler_remove_thread(Current_Thread);
    lock_release_writer(&Current_Thread_Lock);
}


void scheduler_yield(void) {
    if (Current_Thread == NULL) {
        // FIXME: Reaches into private APIs for no good reason.
        //pic_end_of_interrupt(0x20);
        interrupts_enable();
        cpu_sleep_state();
    } else if (Current_Thread != Current_Thread->next) {
        logf(Log_Debug, "Switching from process %d:%d to %d:%d\n",
            Current_Thread->process->id, Current_Thread->id,
            Current_Thread->next->process->id, Current_Thread->next->id);
        lock_acquire_writer(&Current_Thread_Lock);
        struct thread *prev = Current_Thread;
        Current_Thread = Current_Thread->next;
        lock_release_writer(&Current_Thread_Lock);
        thread_switch(prev, Current_Thread);
    }
}


struct process *scheduler_get_current_process(void) {
    struct process *process;
    lock_acquire_reader(&Current_Thread_Lock);
    if (Current_Thread != NULL) {
        process = Current_Thread->process;
        object_retain(&process->obj);
    } else {
        process = NULL;
    }
    lock_release_reader(&Current_Thread_Lock);
    return process;
}


struct thread *scheduler_get_current_thread(void) {
    lock_acquire_reader(&Current_Thread_Lock);
    struct thread *thread = Current_Thread;
    object_retain(&thread->obj);
    lock_release_reader(&Current_Thread_Lock);
    return thread;
}


void scheduler_add_thread(struct thread *thread) {
    assert(thread != NULL);
    lock_acquire_writer(&Current_Thread_Lock);
    if (Current_Thread != NULL) {
        thread->next = Current_Thread->next;
        thread->prev = Current_Thread;
        Current_Thread->next->prev = thread;
        Current_Thread->next = thread;
    } else {
        Current_Thread = thread;
        thread->next = thread;
        thread->prev = thread;
    }
    object_retain(&thread->obj);
    lock_release_writer(&Current_Thread_Lock);
}


void scheduler_remove_thread(struct thread *thread) {
    assert(thread != NULL);
    lock_acquire_writer(&Current_Thread_Lock);
    if (thread->next == thread) {
        Current_Thread = NULL;
    } else {
        thread->prev->next = thread->next;
        thread->next->prev = thread->prev;
    }
    thread->next = NULL;
    thread->prev = NULL;
    object_release(&thread->obj);
    lock_release_writer(&Current_Thread_Lock);
}
