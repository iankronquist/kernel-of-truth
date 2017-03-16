#include <arch/x64/control_registers.h>
#include <arch/x64/interrupts.h>
#include <arch/x64/paging.h>
#include <arch/x64/segments.h>

#include <truth/panic.h>
#include <truth/process.h>
#include <truth/slab.h>
#include <truth/string.h>

extern void _privilege_level_switch(void *rip, uint16_t code_segment,
                                    void *rsp, uint16_t data_segment);


uint64_t read_rsp(void);
extern void _thread_switch(uint64_t *new_stack, uint64_t **old_stack,
                           uint64_t cr3);

void thread_switch(struct thread *old_thread,
                          struct thread *new_thread) {
    assert(new_thread != old_thread);
    tss_set_stack((uint8_t *)new_thread->kernel_stack +
                  new_thread->kernel_stack_size);
    _thread_switch(new_thread->current_stack_pointer,
            &old_thread->current_stack_pointer,
            new_thread->process->page_table->physical_address);
}

enum status thread_user_stack_init(struct thread *thread, struct process *proc,
                                   void *entry_point) {
    assert(thread != NULL);
    assert(proc != NULL);
    assert(proc->address_space != NULL);
    assert(proc->page_table != NULL);

    enum status status;
    uint16_t code_segment;
    uint16_t data_segment;
    phys_addr original_cr3 = read_cr3();
    page_table_switch(proc->page_table->physical_address);
    phys_addr phys;
    thread->user_stack_size = Thread_Default_User_Stack_Size;
    thread->user_stack = slab_alloc_helper(thread->user_stack_size, &phys,
                                           Memory_Writable |
                                               Memory_User_Access,
                                           proc->address_space);
    if (thread->user_stack == NULL) {
        status = Error_No_Memory;
        goto out;
    }

    thread->current_stack_pointer = (uint64_t *)(
        (uint8_t *)thread->kernel_stack + thread->kernel_stack_size -
        sizeof(struct interrupt_cpu_state) -
        sizeof(uint64_t));
    memset(thread->kernel_stack, 0, thread->kernel_stack_size);
    memset(thread->user_stack, 0xdddddddd, thread->user_stack_size);
    struct interrupt_cpu_state *state = (void *)thread->current_stack_pointer;

    if (thread->user_space) {
        code_segment = Segment_User_Code | Segment_RPL;
        data_segment = Segment_User_Data | Segment_RPL;
    } else {
        code_segment = Segment_Kernel_Code;
        data_segment = Segment_Kernel_Data;
    }

    // Set up for _privilege_level_switch
    // The AMD64 ABI specifies the first four integer arguments are:
    // rdi, rsi, rdx, rcx
    state->rdi = (uintptr_t)entry_point;
    state->rsi = code_segment;
    state->rdx = (uintptr_t)((uint8_t *)thread->user_stack +
                             thread->user_stack_size);
    state->rcx = data_segment;

    uint64_t *rip = (uint64_t*)((uint8_t *)thread->kernel_stack +
                                thread->kernel_stack_size -
                                sizeof(uint64_t) * 8);
    *rip = (uintptr_t)_privilege_level_switch;

    status = Ok;
out:
    page_table_switch(original_cr3);
    return status;
}

