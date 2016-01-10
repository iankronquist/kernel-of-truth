#include <arch/x86/process.h>

uint32_t get_next_pid() {
    static uint32_t proc_count = 0;
    proc_count++;
    return proc_count;
}

void proc_init() {
    Cur_Proc = kmalloc(sizeof(struct process));
    memset(Cur_Proc, 0, sizeof(struct process));
    Cur_Proc->next = Cur_Proc;
    Cur_Proc->id = get_next_pid();
}

struct cpu_state *create_proc(void(*entrypoint)()) {
    struct process *new_proc = kmalloc(sizeof(struct process));
    new_proc->id = get_next_pid();

    // Declare some regions of memory we will use for the new process
    // These are all virtual addresses on a new page table so we won't have to
    // worry about collisions too much.
    uint32_t link_loc = 0x20000;
    uint32_t stack_page = link_loc - PAGE_SIZE;
    uint32_t stack_start = link_loc - 1;


    // Set up pointers to memory we're going to use.

    // The stack pointer should point to the top of the registers we set up on
    // top of the stack.
    new_proc->esp = stack_start + 40;
    new_proc->eip = (uint32_t)entrypoint;
    set_up_stack(new_proc->esp, new_proc->eip);

    // Insert the new process into the circularly linked list.
    new_proc->next = Cur_Proc->next->next;
    Cur_Proc->next = new_proc;
    return new_proc;
}

void preempt() {
    Cur_Proc = Cur_Proc->next;
    enter_proc(Cur_Proc->esp);
}
