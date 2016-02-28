#include <arch/x86/idt.h>
#include <drivers/keyboard.h>

struct idt_entry idt[256];
struct idt_ptr idtp;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt[num].base_lo = base & 0xffff;
    idt[num].base_hi = (base >> 16) & 0xffff;
    idt[num].always0 = 0;
    idt[num].sel = sel;
    idt[num].flags = flags;
}

void idt_install()
{
    // 256 is the number of entries in the table.
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t) & idt;

    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8e);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8e);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8e);
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8e);
    idt_set_gate(4, (uint32_t)isr4, 0x08, 0x8e);
    idt_set_gate(5, (uint32_t)isr5, 0x08, 0x8e);
    idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8e);
    idt_set_gate(7, (uint32_t)isr7, 0x08, 0x8e);
    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8e);
    idt_set_gate(9, (uint32_t)isr9, 0x08, 0x8e);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8e);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8e);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8e);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8e);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8e);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8e);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8e);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8e);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8e);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8e);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8e);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8e);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8e);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8e);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8e);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8e);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8e);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8e);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8e);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8e);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8e);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8e);

    idt_set_gate(32, (uint32_t)timer_handler, 0x08, 0x8e);
    idt_set_gate(33, (uint32_t)keyboard_handler, 0x08, 0x8e);

    // There are 4 Interrupt Command Word Registers and I'm not entirely sure
    // what they do. I can only find a brief mention of them in section
    // 33.3.2.1 of the IA-32 Manual. More reading is necessary.
    // ICW1 - begin initialization
    write_port(PIC_MASTER_CONTROL, 0x11);
    write_port(PIC_SLAVE_CONTROL, 0x11);

    // Remap interrupts beyond 0x20 because the first 32 are cpu exceptions
    write_port(PIC_MASTER_MASK, 0x21);
    write_port(PIC_SLAVE_MASK, 0x28);

    // ICW3 - setup cascading
    write_port(PIC_MASTER_MASK, 0x00);
    write_port(PIC_SLAVE_MASK, 0x00);

    // ICW4 - environment info
    write_port(PIC_MASTER_MASK, 0x01);
    write_port(PIC_SLAVE_MASK, 0x01);

    // mask interrupts
    write_port(PIC_MASTER_MASK, 0xff);
    write_port(PIC_SLAVE_MASK, 0xff);

    idt_load((uint32_t) & idtp);
}

void common_interrupt_handler(struct regs r)
{
    sys_kprintf("Interrupt Triggered!\nRegisters:");
    sys_kprintf("ds: %p edi: %p esi: %p ebp: %p esp: %p ebx: %p edx: %p ecx: %p eax: %p int_no: %p err_code: %p eip: %p cs: %p eflags: %p useresp: %p ss: %p", r);

    kabort();
}
