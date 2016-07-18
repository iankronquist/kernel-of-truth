#include <stddef.h>
#include <stdint.h>
#include <string.h>

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
#include <libk/modloader.h>

/* This little function exists to demonstrate the multi-processing
 * functionality. It spins and logs its progress. It takes no arguments and
 * does not return.
*/
void worker(void) {
    while(1) {
        klog("worker\n");
    }
}

// Heap allocated kernel command line.
static char *command_line = NULL;
// Size of the heap allocated kernel command line.
static size_t command_line_size = 0;

/* Copy the multiboot command line into heap allocated memory.
 * Uses kmalloc, so it must be run after the heap is installed.
 */
static void copy_command_line(struct multiboot_info *mb) {
    command_line_size = strnlen((char*)mb->cmdline, PAGE_SIZE);
    command_line = kmalloc(command_line_size);
    strncpy(command_line, (char*)mb->cmdline, PAGE_SIZE);
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
void kernel_main(struct multiboot_info *mb) {
    gdt_install();
    idt_install();
    terminal_initialize();
    initialize_klog();
    kheap_install((struct kheap_metadata*)KHEAP_PHYS_ROOT, PAGE_SIZE);
    copy_command_line(mb);
    persist_multiboot_module_data(mb);
    physical_allocator_init(mb->mem_upper + mb->mem_lower);
    (void)kernel_page_table_install(mb);
    // Multiboot tables are not paged. Nullify the pointer so nobody tries to
    // access it agaion.
    mb = NULL;

    keyboard_install();
    char *hi = "Hello kernel!\n";
    void *testing = kmalloc(16);
    memcpy(testing, hi, 16);
    kputs(testing);
    klog(testing);
    kfree(testing);

    proc_init();
    struct process *worker_proc = create_proc(worker);
    schedule_proc(worker_proc);

    while (1) {
        klog("kernel\n");
    }
}

