#include <truth/memory.h>
#include <truth/types.h>
#include <arch/x64/segments.h>

struct tss_entry Tss = {
    .reserved0 = 0,
    .stack0 = 0,
    .stack1 = 0,
    .stack2 = 0,
    .reserved1 = 0,
    .ist = { 0, 0, 0, 0, 0, 0, 0},
    .reserved2 = 0,
    .reserved3 = 0,
    .iomap_base = 0,
};

struct gdt_entry GDT[] = {
    // NULL segment
    gdt_entry(0, 0, 0, 0),

    // Kernel Code Segment.
    gdt_entry(0, 0xffffffff, 0x9a, Gran_64_Bit_Mode | Gran_4_KB_Blocks),

    // Kernel Data Segment.
    gdt_entry(0, 0xffffffff, 0x92, Gran_64_Bit_Mode | Gran_4_KB_Blocks),

    // User Code Segment.
    gdt_entry(0, 0xffffffff, 0xfa, Gran_64_Bit_Mode | Gran_4_KB_Blocks),

    // User Data Segment.
    gdt_entry(0, 0xffffffff, 0xf2, Gran_64_Bit_Mode | Gran_4_KB_Blocks),

    // Task Switch Segment.
    gdt_entry64(0ull, sizeof(Tss) - 1, 0xe9, 0x00),
};


struct gdt_register Physical_GDT_Register = {
    .limit = sizeof(GDT) - 1,
    .base = virt_to_phys((uint64_t)&GDT),
};

struct gdt_register GDT_Register = {
    .limit = sizeof(GDT) - 1,
    .base = (uint64_t)&GDT,
};

void tss_set_stack(void *stack) {
    Tss.stack0 = (uintptr_t)stack;
}
