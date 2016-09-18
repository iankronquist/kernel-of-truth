#include <truth/cpu.h>
#include <truth/types.h>
#include <truth/log.h>
#include <truth/physical_allocator.h>

void kernel_main(void *multiboot_tables) {
    init_interrupts();
    enum status unused(status) = init_log("log");
    init_physical_allocator(multiboot_tables);
    logf("The Kernel of Truth\n\tVersion %d.%d.%d\n\tCommit %s\n", kernel_major, kernel_minor, kernel_patch, vcs_version);
    halt();
}
