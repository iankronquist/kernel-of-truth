#include <arch/x86/idt.h>
#include <drivers/keyboard.h>

// Definitions of locations of the PIC ports.
// The Programmable Interrupt Controller, or PIC, has two parts, the master and
// the slave.
#define PIC_MASTER_CONTROL 0x20
#define PIC_MASTER_MASK 0x21
#define PIC_SLAVE_CONTROL 0xa0
#define PIC_SLAVE_MASK 0xa1

/* The Interrupt Descriptor Table and its entries.
 * The Interrupt Descriptor Table, or IDT, describes the code called when an
 * interrupt occurs.
 * It has a few important fields:
 * 
 * @base_lo: The lower half of a pointer to the code which will be called when
 * the interrupt is triggered.
 * @base_hi: The higher half of that same pointer.
 * @sel: The interrupt routine will be called with this <gdt> segment.
 * @always0: Reserved.
 * @flags: Contains the present flag, as well as the Descriptor Privilege
 * Level. The DPL indicates what ring code needs to be running as in order to
 * issue this interrupt, or in our case whether use mode code can issue this
 * interrupt.
 * For more information, read Volume 3 Chapter 6 of the Intel Manual.
 */
static struct idt_entry {
    uint16_t base_lo;
    uint16_t sel; // Kernel segment goes here.
    uint8_t always0;
    uint8_t flags; // Set using the table.
    uint16_t base_hi;
} __attribute((packed)) idt[256];

/*
 * Similar to the <gdt_ptr>, this is a special pointer to the <idt>.
 */
static struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute((packed)) idtp;


/* A wrapper around lidt.
 * Load the provided <idt_ptr> onto the CPU.
 */
extern void idt_load(struct idt_ptr);

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr32();

extern void keyboard_handler();

/* Set an entry in the <idt>.
 */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = base & 0xffff;
    idt[num].base_hi = (base >> 16) & 0xffff;
    idt[num].always0 = 0;
    idt[num].sel = sel;
    idt[num].flags = flags;
}

/* Initialize the <idt> and the 8259 Programmable Interrupt Controller.
 * There are actually two PICs, a master and a slave. Each is controlled via a
 * dedicated I/O port. We remap interrupts so that we can catch CPU exceptions.
 * Interrupts 0 through 31 are CPU exceptions and currently get sent to the
 * <common_interrupt_handler>. Interrupt 32 is used by the programmable timer
 * which dispatches the <timer_handler>. Interrupt 33 is used by the keyboard,
 * and dispatches the <keyboard_handler>.
 */
void idt_install(void) {
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

    idt_load(idtp);
}

/* Print out the state of the CPU and kernel panic */
void common_interrupt_handler(struct regs r) {
    kprintf("Interrupt Triggered!\nRegisters:");
    kprintf("ds: %u edi: %u esi: %u ebp: %u esp: %u ebx: %u edx: %u ecx: %u eax: %u int_no: %u err_code: %u eip: %u cs: %u eflags: %u useresp: %u ss: %u", r);

    kabort();
}
