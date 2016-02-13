#include <libk/kabort.h>

void kabort()
{
    __asm__ volatile ("cli");
    // TODO: Add proper kernel panic.
    sys_kputs("\nKernel Panic! Aborting!\n");
    while (1) {
    }
}
