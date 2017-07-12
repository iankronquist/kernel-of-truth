#include <loader/jitter.h>
#include <truth/memory.h>

#define Boot_Jitter_SHA1_Starting_Values 0xefcdab8967452301

#define Boot_Jitter_Max_Fold_Bits 4
#define Boot_Jitter_Buffer_Size (2 * Page_Small)
#define Boot_Jitter_Fold_Mask (0xff)

static inline uint64_t boot_cpu_get_ticks(void) {
    uint32_t eax, edx;
    __asm__ volatile ("rdtsc" : "=(eax)"(eax), "=(edx)"(edx)::);
    return (((uint64_t)edx) << 32) | eax;
}

uint64_t boot_memory_jitter_calculate(void) {
    uint8_t *memory = boot_allocator(Boot_Jitter_Buffer_Size/Page_Small);
    uint64_t entropy = Boot_Jitter_SHA1_Starting_Values;
    for (size_t i = 0; i < Boot_Jitter_Buffer_Size; ++i) {
        uint64_t before = boot_cpu_get_ticks();
        memory[i] += 1;
        uint64_t after = boot_cpu_get_ticks();
        uint64_t delta = after - before;
        entropy ^= delta & Boot_Jitter_Fold_Mask;
        entropy = (entropy << 8) | (entropy & 0xff00000000000000) >> 56;
    }
    return entropy;
}
