#include <libk/proc.h>

struct process_table *scheduling_queue = NULL;

true_pid_t pid_counter = 0;

true_pid_t get_next_pid() {
    true_pid_t new_pid = pid_counter;
    ++pid_counter;
    return new_pid;
}

void scheduler_wakeup(struct regs r) {
    kputs("wakeup");
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
    kputs("start");
    struct process *proc = kmalloc(sizeof(struct process));
    proc->id = get_next_pid();
    proc->directory = create_new_page_table(kernel_pages);
    enable_paging(proc->directory);
    entrypoint();
}

void resume_proc(struct process *resume) {
    kputs("resume");
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



