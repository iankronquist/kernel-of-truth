#include <arch/x86/process.h>

uint32_t get_next_pid() {
    static uint32_t proc_count = 0;
    return proc_count++;
}

struct process *running_proc;

void proc_init() {
    struct process *kernel_proc = kmalloc(sizeof(struct process));
    kernel_proc->next = kernel_proc;
    kernel_proc->regs.cr3 = get_page_dir();
    klogf("kernel page dir: %p\n", kernel_proc->regs.cr3);
    running_proc = kernel_proc;
}

struct process *create_proc(void(*entrypoint)()) {
    uint32_t *link_loc = 0x20000;
    uint32_t stack_page = (uint32_t)NEXT_PAGE(link_loc);
    uint32_t stack_addr = stack_page + PAGE_SIZE-1;
    struct process *proc = kmalloc(sizeof(struct process));
    memset(&proc->regs, 0, sizeof(struct registers));
    proc->regs.eflags = get_flags();
    proc->regs.eip = (uint32_t)entrypoint;

    proc->regs.cr3 = create_new_page_dir(get_page_dir(), link_loc, stack_page,
            PAGE_USER_MODE | PAGE_WRITABLE);

    klogf("worker page dir: %p\n", proc->regs.cr3);
    proc->regs.esp = stack_addr;
    proc->id = get_next_pid();
    proc->next = 0;
    return proc;
}

void schedule_proc(struct process *proc) {
    proc->next = running_proc->next;
    running_proc->next = proc;
}

void preempt() {
    struct process *last = running_proc;
    // If the next proc is the same as the current one then exit
    running_proc = running_proc->next;
    klogf("next cr3 %p\n", running_proc->regs.cr3);
    klogf("next esp %p\n", running_proc->regs.esp);
    klogf("next ebp %p\n", running_proc->regs.ebp);
    switch_task(&last->regs, &running_proc->regs);
}
