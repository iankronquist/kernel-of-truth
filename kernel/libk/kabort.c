#include <stdarg.h>
#include <stdbool.h>
#include <truth/interrupts.h>
#include <truth/kabort.h>
#include <truth/kputs.h>

#ifdef __i386__
#include <arch/x86/cpu.h>
#elif __arm__
#include <arch/arm/cpu.h>
#endif

void kabort(void) {
    disable_interrupts();
    kputs("\nKernel Panic!\n");
    while (true) {
        halt();
    }
}

void kabort_message(char *format, ...) {
    va_list args;
    va_start(args, format);
    disable_interrupts();
    kputs("\nKernel Panic! Aborting!\n");
    kvprintf(format, args);
    while (true) {
        halt();
    }
    va_end(args);
}
