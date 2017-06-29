.text
.code64

#define CPU_Flag_Interrupts (1 << 9)


# void _privilege_level_switch(void *rip, uint16_t code_segment, void *rsp,
#                              uint16_t data_segment);
.global _privilege_level_switch
.type _privilege_level_switch, @function
_privilege_level_switch:

    # Data segments
    mov %cx, %ds
    mov %cx, %es
    mov %cx, %fs
    mov %cx, %gs

    push %rcx                 # ss
    push %rdx                 # rsp
    push $CPU_Flag_Interrupts # rflags
    push %rsi                 # cs
    push %rdi                 # rip

    iretq


# void _thread_switch(uint64_t *new_stack, uint64_t **old_stack, uint64_t cr3);
.global _thread_switch
.type _thread_switch, @function
_thread_switch:
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rbp
    push %rsi
    push %rdi
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15

    pushf

    mov %rsp, (%rsi)

    mov %rdx, %cr3

    mov  %rdi, %rsp

    popf

    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rdi
    pop %rsi
    pop %rbp
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax
    sti
    ret


# struct thread *thread_get_data(void)
.global thread_get_data
.type thread_get_data, @function
thread_get_data:
    movq %gs:Thread_Self_GS_Offset, %rax
    ret


# void thread_set_data(struct thread *)
.global thread_set_data
.type thread_set_data, @function
thread_set_data:
    movq %rsi, %rax
    movq %rsi, %rdx
    shlq $32, %rdx
    movl $MSR_GS_BASE, %ecx
    wrmsr
    ret
