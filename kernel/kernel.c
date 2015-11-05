#include <stddef.h>
#include <stdint.h>
#include "./arch/x86/idt.h"
#include <terminal.h>
#include <kabort.h>
#include <kassert.h>

 
void kernel_main() {
    term_initialize();
    idt_install();

    kputs("Interrupt?");
    int i = 0;
    int b = 128;
    int d = 1;

    d = b/i;

    while(1);
}
