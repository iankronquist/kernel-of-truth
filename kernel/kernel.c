#include <stddef.h>
#include <stdint.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/io.h>
#include <drivers/terminal.h>
#include <drivers/keyboard.h>
#include <drivers/timer.h>

#include <libk/kmem.h>
#include <libk/kputs.h>

void kernel_main()
{
    term_initialize();
    gdt_install();
    idt_install();
    keyboard_install();
    kheap_install();
    // Periodically prints 'tick!' on the screen. This will be useful later for
    // multi-tasking.
    // timer_install();

    kputs("Hello kernel!");
    void *testing = kmalloc(100);

    while (1) {
    }
}
