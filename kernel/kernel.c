#include <stddef.h>
#include <stdint.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/io.h>
#include <arch/x86/paging.h>
#include <arch/x86/process.h>
#include <drivers/terminal.h>
#include <drivers/keyboard.h>
#include <drivers/timer.h>

#include <libk/kmem.h>
#include <libk/kputs.h>
#include <libk/klog.h>

void worker() {
    while(1) {
        klog("worker\n");
        preempt();
    }
}

void kernel_main()
{
    terminal_initialize();
    initialize_klog();
    gdt_install();
    idt_install();
    keyboard_install();
    kheap_install((struct kheap_metadata*)KHEAP_PHYS_ROOT, PAGE_SIZE);
    // Periodically prints 'tick!' on the screen. This will be useful later for
    // multi-tasking.
    //timer_install();
    char *hi = "Hello kernel!\n";
    void *testing = kmalloc(16);
    memcpy(testing, hi, 16);
    kputs(testing);
    klog(testing);
    kfree(testing);
    uint32_t *page_dir = kernel_page_table_install();

    proc_init();
    struct process *worker_proc = create_proc(worker);
    schedule_proc(worker_proc);


    while (1) {
        klog("kernel\n");
        preempt();
    }
}

