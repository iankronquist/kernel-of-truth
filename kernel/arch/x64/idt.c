#include <truth/cpu.h>
#include <truth/lock.h>
#include <truth/panic.h>
#include <truth/types.h>

#include <arch/x64/idt.h>
#include <arch/x64/isr.h>
#include <arch/x64/segments.h>
#include <arch/x64/pic.h>
#include <arch/x64/port.h>

isr_f ISRs[] = {
    isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7, isr8, isr9, isr10, isr11,
    isr12, isr13, isr14, isr15, isr16, isr17, isr18, isr19, isr20, isr21,
    isr22, isr23, isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31,
    isr32, isr33, isr34,
};

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
    uint64_t base;
} pack;

extern void idt_load(struct idt_ptr *);

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
    for (size_t i = 0; i < static_array_count(ISRs); ++i) {
        idt_set_gate(i, (uintptr_t)ISRs[i], Segment_Kernel_Code, 0x8e);
    }

    struct idt_ptr idtp;
    idtp.limit = (sizeof(struct idt_entry) * IDT_Size) - 1;
    idtp.base = (uintptr_t)&IDT;
    idt_load(&idtp);
}

void idt_fini(void) {
    interrupts_disable();
}
