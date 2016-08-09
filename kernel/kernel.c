#include <string.h>

#include <arch/x86/gdt.h>
#include <arch/x86/io.h>
#include <arch/x86/paging.h>
#include <arch/x86/process.h>

#include <contrib/multiboot.h>

#include <drivers/terminal.h>
#include <drivers/keyboard.h>
#include <drivers/timer.h>

#include <truth/interrupts.h>
#include <truth/kmem.h>
#include <truth/kputs.h>
#include <truth/klog.h>
#include <truth/physical_allocator.h>
#include <truth/types.h>

#include <truth/private/init.h>

/* This little function exists to demonstrate the multi-processing
 * functionality. It spins and logs its progress. It takes no arguments and
 * does not return.
*/
void worker(void) {
    while(1) {
        klog("worker\n");
    }
}

/* This is the entry point to the bulk of the kernel.
 * @mb is a pointer to a <multiboot_info> structure which has important
 * information about the layout of memory which is passed along from the
 * bootloader.
 * First, the kernel sets up the <gdt> or Global Descriptor Table which
 * describes the layout of memory.
 * Next, it installs the <idt> or interrupt descriptor table, which declares
 * which interrupts are available, and whether they can be accessed from user
 * mode.
 * After the basic CPU state has been initialized, logging and the VGA terminal
 * are initialized, and the kernel heap is installed. Next, the physical memory
 * allocator and the keyboard drivers are initialized. Note that the kernel
 * heap must be initialized before the physical memory allocator since certain
 * parts of the allocator live on the heap. Finally the init process page table
 * is created, and multi-processing is initialized.
 * @return this function should never return.
*/
void kernel_main(void *unused(multiboot_tables)) {
    init_cpu();
    init_interrupts();
    terminal_initialize();
    init_logging();
    init_memory(multiboot_tables);
    keyboard_install();
    char *hi = "Hello kernel!\n";
    void *testing = kmalloc(16);
    memcpy(testing, hi, 16);
    kputs(testing);
    klog(testing);
    kfree(testing);

    init_multitasking();
    struct process *worker_proc = create_proc(worker);
    schedule_proc(worker_proc);

    while (1) {
        klog("kernel\n");
    }
}

