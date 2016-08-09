#pragma once

// Halt the CPU.
static inline void halt(void) {
    __asm__ volatile ("wfe");
}
