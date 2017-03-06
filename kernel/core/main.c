#include <truth/panic.h>
#include <truth/cpu.h>
#include <truth/types.h>
#include <truth/log.h>
#include <truth/slab.h>
#include <truth/heap.h>
#include <truth/physical_allocator.h>

const char *Logo = "\n"
                   "            _.-.\n"
                   "        .-.  `) |  .-.\n"
                   "    _.'`. .~./  \\.~. .`'._\n"
                   " .-' .'.'.'.-|  |-.'.'.'. '-.\n"
                   "  `'`'`'`'`  \\  /  `'`'`'`'`\n"
                   "             /||\\\n"
                   "            //||\\\\\n"
                   "\n"
                   "      The Kernel of Truth\n";

uintptr_t ASLR_Offset;
void *read_rip(void);

void kernel_main(void *multiboot_tables) {
    assert_ok(log_init(Log_Warning, "log"));
    log(Log_None, Logo);
    logf(Log_None, "\tVersion %d.%d.%d\n\tCommit %s\n\t%s\n\tCPU Time %ld\n",
         kernel_major, kernel_minor, kernel_patch, vcs_version,
         project_website, cpu_time());
    init_interrupts();
    init_physical_allocator(multiboot_tables);
    init_slab();
    assert_ok(init_heap());
    halt();
}
