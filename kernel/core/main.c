#include <truth/panic.h>
#include <truth/cpu.h>
#include <truth/types.h>
#include <truth/log.h>
#include <truth/slab.h>
#include <truth/heap.h>
#include <truth/physical_allocator.h>

string Logo = str("            _.-.              \n"
                  "        .-.  `) |  .-.        \n"
                  "    _.'`. .~./  \\.~. .`'._   \n"
                  " .-' .'.'.'.-|  |-.'.'.'. '-. \n"
                  "  `'`'`'`'`  \\  /  `'`'`'`'` \n"
                  "             /||\\            \n"
                  "            //||\\\\          \n\n"
                  "      The Kernel of Truth     \n");

void kernel_main(void *multiboot_tables) {
    init_interrupts();
    enum status status = init_log("log");
    assert(status == Ok);
    init_physical_allocator(multiboot_tables);
    init_slab();
    log(Logo);
    logf("\tVersion %d.%d.%d\n\tCommit %s\n\t%s\n",
         kernel_major, kernel_minor, kernel_patch, vcs_version,
         project_website);
    halt();
}
