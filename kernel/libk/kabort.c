#include <stdarg.h>
#include <stdbool.h>
#include <libk/kabort.h>
#include <libk/kputs.h>
#include <arch/x86/idt.h>
#include <arch/x86/cpu.h>

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
