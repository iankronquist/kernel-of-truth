#ifndef CPU_STATE_H
#define CPU_STATE_H

struct cpu_state {
    // Pushed by pusha
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t eip, cs, eflags, useresp, ss;
};
#endif
