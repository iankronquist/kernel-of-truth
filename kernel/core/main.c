#include <truth/panic.h>
#include <truth/cpu.h>
#include <truth/types.h>
#include <truth/log.h>
#include <truth/slab.h>
#include <truth/heap.h>
#include <truth/physical_allocator.h>

string Logo = str("\n"
                  "            _.-.\n"
                  "        .-.  `) |  .-.\n"
                  "    _.'`. .~./  \\.~. .`'._\n"
                  " .-' .'.'.'.-|  |-.'.'.'. '-.\n"
                  "  `'`'`'`'`  \\  /  `'`'`'`'`\n"
                  "             /||\\\n"
                  "            //||\\\\\n"
                  "\n"
                  "      The Kernel of Truth\n");

void kernel_main(void *multiboot_tables) {
    assert_ok(init_log("log"));
    log(Logo);
    logf("\tVersion %d.%d.%d\n\tCommit %s\n\t%s\n\tCPU Time %ld\n",
         kernel_major, kernel_minor, kernel_patch, vcs_version,
         project_website, cpu_time());
    init_interrupts();
    init_physical_allocator(multiboot_tables);
    init_slab();
    assert_ok(init_heap());
    assert_ok(init_modules(multiboot_tables));
    halt();
}
