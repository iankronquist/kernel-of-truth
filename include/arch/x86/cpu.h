#pragma once

#include <truth/types.h>

/* The state of the CPU when an interrupt is triggered. */
struct cpu_state {
    uint32_t ds; /* pushed the segs last */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* pushed by 'pusha' */
    uint32_t int_no, err_code; /* our 'push byte #' and ecodes do this */
    uint32_t eip, cs, eflags, useresp, ss; /* pushed by the processor automatically */
};

// Halt the CPU.
static inline void halt(void) {
    __asm__ volatile ("hlt");
}
