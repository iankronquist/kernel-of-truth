#include <stddef.h>
#include <stdint.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/io.h>
#include <arch/x86/paging.h>
#include <arch/x86/process.h>
#include <contrib/multiboot.h>
#include <drivers/terminal.h>
#include <drivers/keyboard.h>
#include <drivers/timer.h>


#include <libk/kmem.h>
#include <libk/kputs.h>
#include <libk/klog.h>

void worker() {
    while(1) {
        klog("worker\n");
    }
}



void kernel_main(struct multiboot_info *mb) {
    gdt_install();
    idt_install();
    terminal_initialize();
    initialize_klog();
    klogf("mb %p\n", mb);
    kheap_install((struct kheap_metadata*)KHEAP_PHYS_ROOT, PAGE_SIZE);
    physical_allocator_init(mb->mem_upper + mb->mem_lower);
    keyboard_install();
    char *hi = "Hello kernel!\n";
    void *testing = kmalloc(16);
    memcpy(testing, hi, 16);
    kputs(testing);
    klog(testing);
    kfree(testing);
    (void)kernel_page_table_install(mb);

    proc_init();
    struct process *worker_proc = create_proc(worker);
    schedule_proc(worker_proc);


    while (1) {
        klog("kernel\n");
    }
}

