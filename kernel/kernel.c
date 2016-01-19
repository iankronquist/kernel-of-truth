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
    for (int i = 0; i < 100; ++i) {
        klog("worker\n");
    }
    spawn(worker);
    exit();
}

void user_worker() {
    for (int i = 0; i < 100; ++i) {
        klog("user worker\n");
    }
    spawn(worker);
    exit();
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
    (void)kernel_page_table_install();
    char *hi = "Hello kernel!\n";
    void *testing = kmalloc(16);
    memcpy(testing, hi, 16);
    sys_kputs(testing);
    sys_klog(testing);
    kfree(testing);

    proc_init();
    spawn(worker);
    spawn(user_worker);
    exit();

    while (1) {
        sys_klog("kernel\n");
    }
}

