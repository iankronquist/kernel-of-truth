#include <libk/proc.h>

struct process_table *scheduling_queue = NULL;

true_pid_t pid_counter = 0;

true_pid_t get_next_pid() {
    true_pid_t new_pid = pid_counter;
    ++pid_counter;
    return new_pid;
}

void scheduler_wakeup(struct regs r) {
    kprintf("wu h: %p hn: %p\n", scheduling_queue->head, scheduling_queue->head->next);
    scheduling_queue->head->state = r;
    move_head_to_end(scheduling_queue);
    // Signal end of interrupt
    write_port(0x20, 0x20);

    // resume_proc does not return! Blindly jump into your future.
    // FIXME: This probably leaves some detritus on the stack I should clean up.
    resume_proc(scheduling_queue->head);
}

void init_scheduler(uint32_t *kernel_page_dir) {
    kputs("init");
    scheduling_queue = init_process_table();
    struct process *kernel_main_proc = kmalloc(sizeof(struct process));

    kernel_main_proc->link_loc = KERNEL_START;
    kernel_main_proc->directory = kernel_page_dir;
    // Zero
    kernel_main_proc->id = get_next_pid();
    kernel_main_proc->priority = 0;

    insert_pid(scheduling_queue, kernel_main_proc);

    //set_timer_phase(QUANTUM);

    // FIXME make this have an architecture independent api
    idt_set_gate(32, (uint32_t)process_handler, 0x08, 0x8e);

    uint8_t current_mask = read_port(0x21);
    write_port(0x21 , current_mask & TIMER_INTERRUPT_MASK);
}

// Later on I will change the entry point to a pointer to an elf executable,
// but that requires a loader.
void start_proc(void (*entrypoint)(void)) {
    struct process *proc = kmalloc(sizeof(struct process));
    // Might as well run here. This will be provided by the loader later I
    // believe.
    proc->link_loc = 0x200000;
    proc->id = get_next_pid();
    kprintf("start %d\n", proc->id);

    disable_paging();
    // Law of Demeter violation?
    proc->directory = create_new_page_dir(scheduling_queue->head->directory,
    kernel_pages, proc->link_loc);
    //proc->directory = clone_directory(scheduling_queue->head->directory);
    map_page(proc->directory, proc->link_loc, proc->link_loc, PAGE_PRESENT |
        PAGE_USER_MODE);
    enable_paging(proc->directory);

    //__asm__ volatile ("cli");
    //kabort();
    insert_pid(scheduling_queue, proc);
    entrypoint();
}

void resume_proc(struct process *resume) {
    kprintf("resume %p\n", resume->id);
    enable_paging(resume->directory);

    // Restore interrupts. We should probably call an assembly routine which
    // cleans up the stack here.
    __asm__ volatile ("sti");
    _resume_proc(resume->state.eip, resume->state.ebp, resume->state.esp);
    // NOT REACHED
    kabort();
}

void finish_proc(struct process *finish) {
    kputs("finish");
    free_table(finish->directory);
    remove_pid(scheduling_queue, finish->id);
    kfree(finish);
}



