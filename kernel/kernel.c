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

#include <arch/x86/syscall.h>

#include <libk/kmem.h>
#include <libk/kputs.h>
#include <libk/klog.h>

void worker() {
    while(1) {
        sys_klog("worker\n");
    }
}

void user_worker() {
    while(1) {
        sys_klog("before");
        klog("user worker\n");
        sys_klog("after");
    }
}

void kernel_main()
{
    terminal_initialize();
    initialize_klog();
    gdt_install();
    idt_install();
    install_syscall();
    keyboard_install();
    kheap_install((struct kheap_metadata*)KHEAP_PHYS_ROOT, PAGE_SIZE);
    // Periodically prints 'tick!' on the screen. This will be useful later for
    // multi-tasking.
    //timer_install();
    char *hi = "Hello kernel!\n";
    void *testing = kmalloc(16);
    memcpy(testing, hi, 16);
    kputs(testing);
    sys_klog(testing);
    kfree(testing);
    (void)kernel_page_table_install();

    proc_init();
    struct process *worker_proc = create_proc(worker);
    schedule_proc(worker_proc);

    jump_to_usermode(user_worker);


    while (1) {
        sys_klog("kernel\n");
    }
}

