.text
.code64


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
    ret
