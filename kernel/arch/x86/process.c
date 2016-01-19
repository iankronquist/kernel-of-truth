#include <arch/x86/process.h>

uint32_t get_next_pid() {
    static uint32_t proc_count = 0;
    return proc_count++;
}

struct process *running_proc;

void proc_init() {
    struct process *kernel_proc = kmalloc(sizeof(struct process));
    kernel_proc->id = get_next_pid();
    kernel_proc->next = kernel_proc;
    kernel_proc->prev = kernel_proc;
    kernel_proc->regs.cr3 = get_page_dir();
    running_proc = kernel_proc;

    // FIXME make this have an architecture independent api
    idt_set_gate(32, (uint32_t)_process_handler, 0x08, 0x8e);
    uint8_t current_mask = read_port(0x21);
    write_port(0x21 , current_mask & TIMER_INTERRUPT_MASK);
}

struct process *create_proc(void(*entrypoint)()) {
    uint32_t *link_loc = (uint32_t*)0x20000;
    uint32_t *stack_page = (uint32_t*)NEXT_PAGE(link_loc);
    uint32_t stack_addr = (uint32_t)stack_page + PAGE_SIZE-1;
    struct process *proc = kmalloc(sizeof(struct process));
    memset(&proc->regs, 0, sizeof(struct registers));
    proc->regs.eflags = get_flags();
    proc->regs.eip = (uint32_t)entrypoint;

    proc->regs.cr3 = create_new_page_dir((page_frame_t*)get_page_dir(),
            link_loc, stack_page, PAGE_USER_MODE | PAGE_WRITABLE);

    proc->regs.esp = stack_addr;
    proc->id = get_next_pid();
    proc->next = NULL;
    proc->prev = NULL;
    return proc;
}

void schedule_proc(struct process *proc) {
    proc->next = running_proc->next;
    running_proc->next = proc;
}

void preempt() {
    struct process *last = running_proc;
    running_proc = running_proc->next;
    // switch_task does not behave properly when the last task and current
    // tasks are the same, especially if the state of the current task's
    // registers are not properly initialized.
    // If the next proc is the same as the current one then return.
    if (running_proc == last) {
        return;
    }
    switch_task(&last->regs, &running_proc->regs);
}


void process_handler() {
    // End interrupt
    write_port(0x20, 0x20);
    preempt();
}

void sys_exit() {
    // If there is only one process running and it is trying to exit shut down
    // I guess?
    if (running_proc == running_proc->next) {
        sys_klog("Last process exited. Shutting down.");
        kabort();
    }
    // Deschedule current process.
    running_proc->prev->next = running_proc->next;
    running_proc->next->prev = running_proc->prev;

    // Switch to next process and clean up current one.
    struct process *last = running_proc;
    running_proc = running_proc->next;
    kfree(last);

    // We could write a version of switch task which doesn't save the current
    // machine state, but I'm lazy. Just give it a dummy variable on the stack
    // to put the machine state in instead.
    struct registers dummy;
    switch_task(&dummy, &running_proc->regs);

    // NOT REACHED
    kassert(0);
}

void sys_spawn(void (*entrypoint)(void)) {
    struct process *proc = create_proc(entrypoint);
    sys_klogf("Spawning proc %p\n", proc->id);
    schedule_proc(proc);
}

void sys_exec(void (*entrypoint)(void)) {
    sys_klog("1");
    sys_klogf("execing in proc %p\n", running_proc->id);
    uint32_t *link_loc = (uint32_t*)0x20000;
    sys_klog("2");

    // Reset stack to beginning
    running_proc->regs.esp &= ~0xfff;
    running_proc->regs.ebp &= ~0xfff;
    sys_klog("3");

    //free_table((uint32_t*)running_proc->regs.cr3);
    running_proc->regs.cr3 = create_new_page_dir((page_frame_t*)get_page_dir(),
            link_loc, (uint32_t*)running_proc->regs.esp,
            PAGE_USER_MODE | PAGE_WRITABLE);
    sys_klog("4");

    running_proc->regs.eip = (uint32_t)entrypoint;
    sys_klog("5");
    jump_to_usermode(entrypoint);
}
