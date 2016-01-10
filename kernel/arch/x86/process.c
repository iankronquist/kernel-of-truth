#include <arch/x86/process.h>

uint32_t get_next_pid() {
    static uint32_t proc_count = 0;
    return proc_count++;
}

struct process *running_proc;
struct process kernel_proc;
struct process worker_proc;

void wworker() {
    while(1) {
        kputs("worker");
        preempt();
    }
}

void proc_init() {
    __asm__ volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(kernel_proc.regs.cr3)::"%eax");
    __asm__ volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;":"=m"(kernel_proc.regs.eflags)::"%eax");
    create_proc(&worker_proc, wworker, kernel_proc.regs.eflags,
            (uint32_t*)kernel_proc.regs.cr3);
    kernel_proc.next = &worker_proc;
    worker_proc.next = &kernel_proc;
    running_proc = &kernel_proc;
}

void create_proc(struct process *proc, void(*entrypoint)(), uint32_t flags, uint32_t *page_dir) {
    memset(&proc->regs, 0, sizeof(struct registers));
    proc->regs.eflags = flags;
    proc->regs.eip = (uint32_t)entrypoint;
    proc->regs.cr3 = (uint32_t)page_dir;

    uint32_t *link_loc = 0x20000;
    map_page(kernel_pages, link_loc, link_loc, 0);
    proc->regs.esp = (uint32_t)link_loc;
    proc->id = get_next_pid();
    proc->next = 0;
}

void preempt() {
    struct process *last = running_proc;
    running_proc = running_proc->next;
    switch_task(&last->regs, &running_proc->regs);
}
