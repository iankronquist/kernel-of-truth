#include <truth/panic.h>
#include <truth/cpu.h>
#include <truth/types.h>
#include <truth/log.h>
#include <truth/logo.h>
#include <truth/slab.h>
#include <truth/heap.h>
#include <truth/boot.h>
#include <truth/memory.h>
#include <truth/module.h>
#include <truth/physical_allocator.h>
#include <truth/process.h>
#include <truth/random.h>
#include <truth/timer.h>
#include <arch/x64/paging.h>
#include <truth/device/vga.h>
#include <truth/device/ps2_keyboard.h>
#include <arch/x64/cpu.h>


struct boot_info Boot_Info;


void kernel_main(void) {
    assert_ok(log_init(Log_Debug, "log"));
    log(Log_None, Logo);
    logf(Log_None, "\tCPU Time %ld\n", cpu_time());
    logf(Log_Debug, "Kernel start address: %p\nKernel size: %zu\nMultiboot tables: %p\n", Boot_Info.kernel, Boot_Info.kernel_size, Boot_Info.multiboot_info);
    interrupts_init();
    halt();
    physical_allocator_init(Boot_Info.multiboot_info);
    slab_init();
    logf(Log_Debug, "slab usage %lx\n", slab_get_usage());
    assert_ok(heap_init());
    assert_ok(paging_init());
    memory_init();
    assert_ok(processes_init());
    assert_ok(random_init());
    logf(Log_Debug, "slab usage %lx\n", slab_get_usage());
    assert_ok(modules_init(Boot_Info.multiboot_info));
    timer_init();
    keyboard_init();
    vga_init();
    halt();
}
