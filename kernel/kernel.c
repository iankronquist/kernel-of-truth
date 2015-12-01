#include <stddef.h>
#include <stdint.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/io.h>
#include <arch/x86/paging.h>
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
    kheap_install(KHEAP_PHYS_ROOT, PAGE_SIZE);
    // Periodically prints 'tick!' on the screen. This will be useful later for
    // multi-tasking.
    //timer_install();
    char *hi = "Hello kernel!\n";
    void *testing = kmalloc(16);
    memcpy(testing, hi, 16);
    kputs(testing);
    kfree(testing);



    while (1) {
    }
}
