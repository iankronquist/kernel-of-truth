#include <libk/proc.h>

struct process_table *scheduling_queue = NULL;

true_pid_t pid_counter = 0;

true_pid_t get_next_pid() {
    true_pid_t new_pid = pid_counter;
    ++pid_counter;
    return new_pid;
}

void scheduler_wakeup(struct regs r) {
    /*
    kprintf("wus state %p %p %p\n", r.eip, r.ebp, r.esp);
    kprintf("wu h: %p hn: %p\n", scheduling_queue->head->id, scheduling_queue->head->next->id);
    kprintf("wus n state %p %p %p\n", scheduling_queue->head->next->state.eip, scheduling_queue->head->next->state.ebp, scheduling_queue->head->next->state.esp);
    */
    scheduling_queue->head->state = r;
    move_head_to_end(scheduling_queue);
    // Signal end of interrupt
    write_port(0x20, 0x20);

    // resume_proc does not return! Blindly jump into your future.
    // FIXME: This probably leaves some detritus on the stack I should clean up.
    //resume_proc(scheduling_queue->head);
    enable_paging(scheduling_queue->head->directory);
    r = scheduling_queue->head->state;
}

void init_scheduler(uint32_t *kernel_page_dir) {
    kputs("init");
    scheduling_queue = init_process_table();
    struct process *kernel_main_proc = kmalloc(sizeof(struct process));

    kernel_main_proc->link_loc = KERNEL_START;
    kernel_main_proc->directory = kernel_page_dir;
    //kprintf("kpd %p\n", kernel_page_dir);
    // Zero
    kernel_main_proc->id = get_next_pid();
    kassert(kernel_main_proc->id == 0);
    kernel_main_proc->priority = 0;

    insert_pid(scheduling_queue, kernel_main_proc);

    kassert(scheduling_queue->head == kernel_main_proc);
    kassert(scheduling_queue->head->next == PROCESS_TABLE_END_SENTINEL);
    kassert(scheduling_queue->tail == kernel_main_proc);
    kassert(scheduling_queue->head->id == 0);

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
    // FIXME! this won't always work
    page_frame_t stack_page = proc->link_loc - PAGE_SIZE;
    uint32_t stack_start = proc->link_loc - 1;
    proc->id = get_next_pid();
    memset(&proc->state, 0, sizeof(struct regs));
    proc->state.eip = entrypoint;

    /*
    kprintf("proc state %p %p %p\n", proc->state.eip, proc->state.ebp,
            proc->state.esp);
            */
    disable_paging();
    /*
    proc->directory = create_new_page_dir(scheduling_queue->head->directory,
        kernel_pages, proc->link_loc);
     */
    proc->directory = scheduling_queue->head->directory;
    ///*
    map_page(proc->directory, proc->link_loc, proc->link_loc, PAGE_USER_MODE);
    map_page(proc->directory, stack_page, stack_page, PAGE_USER_MODE);

    proc->state.esp = stack_start;
    proc->state.ebp = stack_start;
    // */
    enable_paging(scheduling_queue->head->directory);


    insert_pid(scheduling_queue, proc);
    move_head_to_end(scheduling_queue);

    //entrypoint();
}

void resume_proc(struct process *resume) {
    /*
    kprintf("resume %p\n", resume->id);
    kprintf("resume dir %p\n", resume->directory);
    kprintf("state %p %p %p\n", resume->state.eip, resume->state.ebp,
            resume->state.esp);
            */
    enable_paging(resume->directory);

    // We should probably call an assembly routine which
    // cleans up the stack here.
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



