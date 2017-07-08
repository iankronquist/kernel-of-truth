#include <arch/x64/cpu.h>
#include <truth/heap.h>
#include <truth/jitter.h>
#include <truth/memory.h>
#include <truth/types.h>

// Very loosely based on this:
// http://www.chronox.de/jent/doc/CPU-Jitter-NPTRNG.html
// https://lwn.net/Articles/642166/

#define Jitter_SHA1_Starting_Values 0xefcdab8967452301

#define Jitter_Max_Fold_Bits 4
#define Jitter_Buffer_Size (2 * KB)
#define Jitter_Fold_Mask (0xff)


uint64_t memory_jitter_calculate(void) {
    uint8_t *memory = kmalloc(Jitter_Buffer_Size);
    uint64_t entropy = Jitter_SHA1_Starting_Values;
    for (size_t i = 0; i < Jitter_Buffer_Size; ++i) {
        uint64_t before = cpu_get_ticks();
        memory[i] += 1;
        uint64_t after = cpu_get_ticks();
        uint64_t delta = after - before;
        entropy ^= delta & Jitter_Fold_Mask;
        entropy = (entropy << 8) | (entropy & 0xff00000000000000) >> 56;
    }
    kfree(memory);
    return entropy;
}
