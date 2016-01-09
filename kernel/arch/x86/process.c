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
    // Important! We will use this to re-enable paging later.
    Cur_Proc->state.cr3 = kernel_pages;
}

struct cpu_state *create_proc(void(*entrypoint)()) {
    struct process *new_proc = kmalloc(sizeof(struct process));
    memset(&new_proc->state, 0, sizeof(struct cpu_state));
    new_proc->id = get_next_pid();

    // Declare some regions of memory we will use for the new process
    // These are all virtual addresses on a new page table so we won't have to
    // worry about collisions too much.
    uint32_t link_loc = 0x20000;
    uint32_t stack_page = link_loc - PAGE_SIZE;
    uint32_t stack_start = link_loc - 1;


    // Set up pointers to memory we're going to use.
    new_proc->state.esp = stack_start;
    new_proc->state.ebp = stack_start;
    new_proc->state.eip = entrypoint;

    // Set up a page table for the new process.
    disable_paging();

    // Create the page table
    new_proc->state.cr3 = create_new_page_dir(Cur_Proc->state.cr3,
        kernel_pages, link_loc);
    // Map the location of the code.
    map_page(new_proc->state.cr3, link_loc, link_loc, PAGE_USER_MODE);
    // Map the location of the data
    map_page(new_proc->state.cr3, stack_page, stack_page, PAGE_USER_MODE);

    // Re-enable paging with the original page table.
    enable_paging(Cur_Proc->state.cr3);

    // Insert the new process into the circularly linked list.
    new_proc->next = Cur_Proc->next->next;
    Cur_Proc->next = new_proc;
    return new_proc;
}

// DO NOT PASS ANY ARGUMENTS TO THIS FUNCTION! DO NOT DECLARE ANY LOCALS!!
void preempt() {
    get_current_regs(&Cur_Proc->state);
    // HACKS AHEAD
    set_eip(Cur_Proc->state.eip);
    Cur_Proc = Cur_Proc->next;
    switch_proc(&Cur_Proc->state);
}
