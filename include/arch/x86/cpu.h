#ifndef CPU_H
#define CPU_H

// Halt the CPU.
static inline void halt(void) {
    __asm__ volatile ("hlt");
}

#endif
