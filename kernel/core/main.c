#include <truth/cpu.h>
#include <truth/types.h>
#include <truth/log.h>
#include <truth/physical_allocator.h>

void kernel_main(void *multiboot_tables) {
    init_interrupts();
    enum status unused(status) = init_log("log");
    log("Hello kernel!");
    init_physical_allocator(multiboot_tables);
    logf("Hello kernel! %p\n", multiboot_tables);
    halt();
}
