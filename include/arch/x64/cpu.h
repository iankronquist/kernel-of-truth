#pragma once

#include <truth/types.h>

#define CPUID_SMAP (1 << 20)
#define CPUID_SMEP (1 << 7)

static inline void cpuid(uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ volatile ("cpuid" :
            "=a"(*eax),
            "=b"(*ebx),
            "=c"(*ecx),
            "=d"(*edx)
            : "0" (*eax), "2" (*ecx)
            :);
}

static inline void cpu_flags_set_ac(void) {
    __asm__ volatile ("stac" ::: "cc");
}

static inline void cpu_flags_clear_ac(void) {
    __asm__ volatile ("clac" ::: "cc");
}

static inline void cpu_cr4_set_bit(int bit) {
    __asm__ volatile ("push %%rax\n"
                      "movq %%cr4, %%rax\n"
                      "orq $0, %%rax\n"
                      "movq %%rax, %%cr4\n"
                      "pop %%rax\n"
            : : "a"(bit));
}

static inline void cpu_cr4_clear_bit(int bit) {
    int mask = ~(1 << bit);
    __asm__ volatile ("andq %0, %%cr4" : : "a"(mask));
}