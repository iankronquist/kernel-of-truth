#include <arch/x86/process.h>

uint32_t get_next_pid() {
    static uint32_t proc_count = 0;
    return proc_count++;
}

struct process *running_proc;

void proc_init() {
    struct process *kernel_proc = kmalloc(sizeof(struct process));
    kernel_proc->next = kernel_proc;
    running_proc = kernel_proc;
}

// FIXME: Split these into their own assembly file
uint32_t get_flags() {
    uint32_t flags;
    __asm__ volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;":"=m"(flags)::"%eax");
    return flags;
}

uint32_t get_page_dir() {
    uint32_t cr3;
    __asm__ volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(cr3)::"%eax");
    return cr3;
}

struct process *create_proc(void(*entrypoint)()) {
    uint32_t *link_loc = 0x20000;
    uint32_t stack_page = (uint32_t)link_loc + PAGE_SIZE;
    struct process *proc = kmalloc(sizeof(struct process));
    memset(&proc->regs, 0, sizeof(struct registers));
    proc->regs.eflags = get_flags();
    proc->regs.eip = (uint32_t)entrypoint;

    map_page(kernel_pages, link_loc, link_loc, 0);

    proc->regs.esp = stack_page;
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
    running_proc = running_proc->next;
    switch_task(&last->regs, &running_proc->regs);
}
