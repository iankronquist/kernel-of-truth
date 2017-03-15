.text
.code64

#define CPU_Flag_Interrupts (1 << 9)


# void _privilege_level_switch(void *rip, uint16_t code_segment, void *rsp, uint16_t data_segment);
.global _privilege_level_switch
.type _privilege_level_switch, @function
_privilege_level_switch:

    mov %cx, %ds
    mov %cx, %es
    mov %cx, %fs
    mov %cx, %gs

    #push %rdi                 # rip
    #push %rsi                 # cs
    #push $CPU_Flag_Interrupts # rflags
    #push %rdx                 # rsp
    #push %rcx                 # ss
    #push %rdi

    push %rcx                 # ss
    push %rdx                 # rsp
    push $CPU_Flag_Interrupts # rflags
    push %rsi                 # cs
    push %rdi                 # rip

    #ret
    iretq


# void _thread_switch(void *new_stack, void **old_stack);
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
