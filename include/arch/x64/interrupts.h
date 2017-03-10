#pragma once

#include <truth/types.h>

struct interrupt_cpu_state {
    uintptr_t ds;
    uintptr_t r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, rdx, rcx,
              rbx, rax;
    uintptr_t interrupt_number, err_code;
    uintptr_t rip, cs, rflags, rsp, ss;
};
