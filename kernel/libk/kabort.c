#include <libk/kabort.h>

void kabort()
{
    // TODO: Add proper kernel panic.
    kputs("\nKernel Panic! Aborting!\n");
    while (1) {
    }
}
