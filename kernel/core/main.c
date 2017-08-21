#include <truth/panic.h>
#include <truth/cpu.h>
#include <truth/types.h>
#include <truth/log.h>
#include <truth/logo.h>
#include <truth/slab.h>
#include <truth/heap.h>
#include <truth/memory.h>
#include <truth/module.h>
#include <truth/physical_allocator.h>
#include <truth/process.h>
#include <truth/random.h>
#include <truth/timer.h>
#include <arch/x64/paging.h>
#include <arch/x64/port.h>
#include <truth/device/vga.h>
#include <truth/device/ps2_keyboard.h>


void kernel_main(uint32_t multiboot_tables) {
    assert_ok(log_init(Log_Debug, "log"));
    log(Log_None, Logo);
    logf(Log_None, "\tCPU Time %ld\n", cpu_time());
    interrupts_init();
    physical_allocator_init(phys_to_virt(multiboot_tables));
    slab_init();
    logf(Log_Debug, "slab usage %lx\n", slab_get_usage());
    assert_ok(heap_init());
    assert_ok(paging_init());
    memory_init();
    assert_ok(processes_init());
    assert_ok(random_init());
    logf(Log_Debug, "slab usage %lx\n", slab_get_usage());
    assert_ok(modules_init(phys_to_virt(multiboot_tables)));
    timer_init();
    keyboard_init();
    vga_init();
#ifdef DEBUG
    log(Log_Debug, "hello");
    test_shutdown_status(Ok);
#endif // DEBUG
    halt();
}
