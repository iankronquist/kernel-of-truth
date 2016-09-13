#include <truth/cpu.h>
#include <truth/types.h>
#include <truth/panic.h>

#include <arch/x64/port.h>

#include "boot.h"

// The state of the CPU when an interrupt is triggered.
struct cpu_state {
    uintptr_t ds;
    uintptr_t rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
    uintptr_t int_no, err_code;
    uintptr_t rip, cs, rflags, useresp, ss;
};

// Definitions of locations of the PIC ports.
// The Programmable Interrupt Controller, or PIC, has two parts, the master and
// the slave.
#define PIC_MASTER_CONTROL 0x20
#define PIC_MASTER_MASK 0x21
#define PIC_SLAVE_CONTROL 0xa0
#define PIC_SLAVE_MASK 0xa1

#define IDT_GATE_PRESENT (17)
#define IDT_SIZE 256

/* The Interrupt Descriptor Table and its entries.
 * The Interrupt Descriptor Table, or IDT, describes the code called when an
 * interrupt occurs.
 * It has a few important fields:
 * 
 * @base_lo: The lower half of a pointer to the code which will be called when
 * the interrupt is triggered.
 * @base_hi: The higher half of that same pointer.
 * @sel: The interrupt routine will be called with this gdt segment.
 * @always0: Reserved.
 * @flags: Contains the present flag, as well as the Descriptor Privilege
 * Level. The DPL indicates what ring code needs to be running as in order to
 * issue this interrupt, or in our case whether use mode code can issue this
 * interrupt.
 * For more information, read Volume 3 Chapter 6 of the Intel Manual.
 */
static struct idt_entry {
    uint16_t base_low;
    uint16_t sel; // Kernel segment goes here.
    uint8_t always0;
    uint8_t flags; // Set using the table.
    uint16_t base_high;
    uint32_t base_highest;
    uint32_t reserved;
} pack Idt[IDT_SIZE] = {{0}};

/*
 * Similar to the gdt_ptr, this is a special pointer to the idt.
 */
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} pack;


// Protects the idt_dispatch_table.
//static spinlock_t idt_dispatch_table_lock = SPINLOCK_INIT;

// The jump table of functions which are called when an interrupt is triggered.
static isr_f *Interrupt_Dispatch[IDT_SIZE] = {0};

/* A wrapper around lidt.
 * Load the provided idt_ptr onto the CPU.
 */
extern void idt_load(struct idt_ptr*);

extern void _service_interrupt(void);

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);
extern void isr32(void);
extern void isr33(void);
extern void isr34(void);

/* Set an entry in the idt.
 */
static void idt_set_gate(uint8_t num, uintptr_t base, uint16_t sel,
        uint8_t flags) {
    Idt[num].base_low = base & 0xffff;
    Idt[num].base_high = (base >> 16) & 0xffff;
    Idt[num].base_highest = base >> 32;
    Idt[num].always0 = 0;
    Idt[num].sel = sel;
    Idt[num].flags = flags;
}

int install_interrupt(uint8_t num, isr_f function) {
    //acquire_spinlock(&idt_dispatch_table_lock);
    if (Interrupt_Dispatch[num] != NULL) {
        return -1;
    }
    Interrupt_Dispatch[num] = function;
    //release_spinlock(&idt_dispatch_table_lock);
    return 0;
}


void init_interrupts(void) {

    /* Initialize the idt and the 8259 Programmable Interrupt Controller.
     * There are actually two PICs, a master and a slave. Each is controlled
     * via a dedicated I/O port. We remap interrupts so that we can catch CPU
     * exceptions.  Interrupts 0 through 31 are CPU exceptions and currently
     * get sent to the common_interrupt_handler. Interrupt 32 is used by
     * the programmable timer which dispatches the timer_handler. Interrupt
     * 33 is used by the keyboard, and dispatches the keyboard_handler.
     */

    idt_set_gate(0, (uintptr_t)isr0, 0x08, 0x8e);
    idt_set_gate(1, (uintptr_t)isr1, 0x08, 0x8e);
    idt_set_gate(2, (uintptr_t)isr2, 0x08, 0x8e);
    idt_set_gate(3, (uintptr_t)isr3, 0x08, 0x8e);
    idt_set_gate(4, (uintptr_t)isr4, 0x08, 0x8e);
    idt_set_gate(5, (uintptr_t)isr5, 0x08, 0x8e);
    idt_set_gate(6, (uintptr_t)isr6, 0x08, 0x8e);
    idt_set_gate(7, (uintptr_t)isr7, 0x08, 0x8e);
    idt_set_gate(8, (uintptr_t)isr8, 0x08, 0x8e);
    idt_set_gate(9, (uintptr_t)isr9, 0x08, 0x8e);
    idt_set_gate(10, (uintptr_t)isr10, 0x08, 0x8e);
    idt_set_gate(11, (uintptr_t)isr11, 0x08, 0x8e);
    idt_set_gate(12, (uintptr_t)isr12, 0x08, 0x8e);
    idt_set_gate(13, (uintptr_t)isr13, 0x08, 0x8e);
    idt_set_gate(14, (uintptr_t)isr14, 0x08, 0x8e);
    idt_set_gate(15, (uintptr_t)isr15, 0x08, 0x8e);
    idt_set_gate(16, (uintptr_t)isr16, 0x08, 0x8e);
    idt_set_gate(17, (uintptr_t)isr17, 0x08, 0x8e);
    idt_set_gate(18, (uintptr_t)isr18, 0x08, 0x8e);
    idt_set_gate(19, (uintptr_t)isr19, 0x08, 0x8e);
    idt_set_gate(20, (uintptr_t)isr20, 0x08, 0x8e);
    idt_set_gate(21, (uintptr_t)isr21, 0x08, 0x8e);
    idt_set_gate(22, (uintptr_t)isr22, 0x08, 0x8e);
    idt_set_gate(23, (uintptr_t)isr23, 0x08, 0x8e);
    idt_set_gate(24, (uintptr_t)isr24, 0x08, 0x8e);
    idt_set_gate(25, (uintptr_t)isr25, 0x08, 0x8e);
    idt_set_gate(26, (uintptr_t)isr26, 0x08, 0x8e);
    idt_set_gate(27, (uintptr_t)isr27, 0x08, 0x8e);
    idt_set_gate(28, (uintptr_t)isr28, 0x08, 0x8e);
    idt_set_gate(29, (uintptr_t)isr29, 0x08, 0x8e);
    idt_set_gate(30, (uintptr_t)isr30, 0x08, 0x8e);
    idt_set_gate(31, (uintptr_t)isr31, 0x08, 0x8e);
    idt_set_gate(32, (uintptr_t)isr32, 0x08, 0x8e);
    idt_set_gate(33, (uintptr_t)isr33, 0x08, 0x8e);
    idt_set_gate(34, (uintptr_t)isr34, 0x08, 0x8e);

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

    struct idt_ptr idtp;
    idtp.limit = (sizeof(struct idt_entry) * IDT_SIZE) - 1;
    idtp.base = (uintptr_t)&Idt;
    __asm__ volatile ("lidt (%0)" : : "r"(&idtp));
}

/* Dispatch event handler or, if none exists, log information and kernel panic. */
void common_interrupt_handler(struct cpu_state r) {
    //assert(r.int_no > IDT_SIZE);
    if (Interrupt_Dispatch[r.int_no] != NULL) {
        Interrupt_Dispatch[r.int_no](&r);
    } else {
        /*
        log("Unhandled Interrupt Triggered!\nRegisters:");
        logf("ds: %p edi: %p esi: %p ebp: %p esp: %p ebx: %p edx: %p ecx: %p "
              "eax: %p int_no: %p err_code: %p eip: %p cs: %p eflags: %p "
              "useresp: %p ss: %p", r);
        panic();
        */
    }
}
