#include <truth/cpu.h>
#include <truth/lock.h>
#include <truth/panic.h>
#include <truth/types.h>

#include <arch/x64/control_registers.h>
#include <arch/x64/isr.h>
#include <arch/x64/segments.h>
#include <arch/x64/pic.h>
#include <arch/x64/port.h>

#define IDT_Size 256

extern void idt_load(struct idt_ptr *);

static struct idt_entry {
    uint16_t base_low;
    uint16_t segment_selector;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_high;
    uint32_t base_highest;
    uint32_t reserved;
} pack IDT[IDT_Size] = {{0}};

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} pack;

/* Set an entry in the idt.  */
static void idt_set_gate(uint8_t num, uintptr_t base,
                         uint16_t segment_selector, uint8_t flags) {
    IDT[num].base_low = base & 0xffff;
    IDT[num].base_high = (base >> 16) & 0xffff;
    IDT[num].base_highest = base >> 32;
    IDT[num].always0 = 0;
    IDT[num].segment_selector = segment_selector;
    IDT[num].flags = flags;
}

void idt_init(void) {

    idt_set_gate(0, (uintptr_t)isr0, Segment_Kernel_Code, 0x8e);
    idt_set_gate(1, (uintptr_t)isr1, Segment_Kernel_Code, 0x8e);
    idt_set_gate(2, (uintptr_t)isr2, Segment_Kernel_Code, 0x8e);
    idt_set_gate(3, (uintptr_t)isr3, Segment_Kernel_Code, 0x8e);
    idt_set_gate(4, (uintptr_t)isr4, Segment_Kernel_Code, 0x8e);
    idt_set_gate(5, (uintptr_t)isr5, Segment_Kernel_Code, 0x8e);
    idt_set_gate(6, (uintptr_t)isr6, Segment_Kernel_Code, 0x8e);
    idt_set_gate(7, (uintptr_t)isr7, Segment_Kernel_Code, 0x8e);
    idt_set_gate(8, (uintptr_t)isr8, Segment_Kernel_Code, 0x8e);
    idt_set_gate(9, (uintptr_t)isr9, Segment_Kernel_Code, 0x8e);
    idt_set_gate(10, (uintptr_t)isr10, Segment_Kernel_Code, 0x8e);
    idt_set_gate(11, (uintptr_t)isr11, Segment_Kernel_Code, 0x8e);
    idt_set_gate(12, (uintptr_t)isr12, Segment_Kernel_Code, 0x8e);
    idt_set_gate(13, (uintptr_t)isr13, Segment_Kernel_Code, 0x8e);
    idt_set_gate(14, (uintptr_t)isr14, Segment_Kernel_Code, 0x8e);
    idt_set_gate(15, (uintptr_t)isr15, Segment_Kernel_Code, 0x8e);
    idt_set_gate(16, (uintptr_t)isr16, Segment_Kernel_Code, 0x8e);
    idt_set_gate(17, (uintptr_t)isr17, Segment_Kernel_Code, 0x8e);
    idt_set_gate(18, (uintptr_t)isr18, Segment_Kernel_Code, 0x8e);
    idt_set_gate(19, (uintptr_t)isr19, Segment_Kernel_Code, 0x8e);
    idt_set_gate(20, (uintptr_t)isr20, Segment_Kernel_Code, 0x8e);
    idt_set_gate(21, (uintptr_t)isr21, Segment_Kernel_Code, 0x8e);
    idt_set_gate(22, (uintptr_t)isr22, Segment_Kernel_Code, 0x8e);
    idt_set_gate(23, (uintptr_t)isr23, Segment_Kernel_Code, 0x8e);
    idt_set_gate(24, (uintptr_t)isr24, Segment_Kernel_Code, 0x8e);
    idt_set_gate(25, (uintptr_t)isr25, Segment_Kernel_Code, 0x8e);
    idt_set_gate(26, (uintptr_t)isr26, Segment_Kernel_Code, 0x8e);
    idt_set_gate(27, (uintptr_t)isr27, Segment_Kernel_Code, 0x8e);
    idt_set_gate(28, (uintptr_t)isr28, Segment_Kernel_Code, 0x8e);
    idt_set_gate(29, (uintptr_t)isr29, Segment_Kernel_Code, 0x8e);
    idt_set_gate(30, (uintptr_t)isr30, Segment_Kernel_Code, 0x8e);
    idt_set_gate(31, (uintptr_t)isr31, Segment_Kernel_Code, 0x8e);
    idt_set_gate(32, (uintptr_t)isr32, Segment_Kernel_Code, 0x8e);
    idt_set_gate(33, (uintptr_t)isr33, Segment_Kernel_Code, 0x8e);
    idt_set_gate(34, (uintptr_t)isr34, Segment_Kernel_Code, 0x8e);

    struct idt_ptr idtp;
    idtp.limit = (sizeof(struct idt_entry) * IDT_Size) - 1;
    idtp.base = (uintptr_t)&IDT;
    idt_load(&idtp);
    interrupts_enable();
}

void idt_fini(void) {
    interrupts_disable();
}
