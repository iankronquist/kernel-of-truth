#include <stddef.h>
#include <stdint.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <drivers/terminal.h>
#include <kabort.h>
#include <kassert.h>

void kernel_main()
{
    term_initialize();
    gdt_install();
    idt_install();

    kputs("Hello kernel!");

    while (1) {
    };
}
