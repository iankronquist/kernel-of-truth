#include <truth/cpu.h>
#include <truth/lock.h>
#include <truth/panic.h>
#include <truth/types.h>

#include <arch/x64/pic.h>
#include <arch/x64/port.h>
#include <arch/x64/control_registers.h>

#define IDT_Size 256

struct cpu_state {
    uintptr_t ds;
    uintptr_t r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, rdx, rcx,
              rbx, rax;
    uintptr_t interrupt_number, err_code;
    uintptr_t rip, cs, rflags, rsp, ss;
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
    uint32_t base;
} pack;


static struct lock idt_dispatch_table_lock = Lock_Clear;

static isr_f *Interrupt_Dispatch[IDT_Size] = {0};

extern void idt_load(struct idt_ptr *);

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
static void idt_set_gate(uint8_t num, uintptr_t base,
                         uint16_t segment_selector, uint8_t flags) {
    IDT[num].base_low = base & 0xffff;
    IDT[num].base_high = (base >> 16) & 0xffff;
    IDT[num].base_highest = base >> 32;
    IDT[num].always0 = 0;
    IDT[num].segment_selector = segment_selector;
    IDT[num].flags = flags;
}

int install_interrupt(uint8_t num, isr_f function) {
    lock_acquire_writer(&idt_dispatch_table_lock);
    if (Interrupt_Dispatch[num] != NULL) {
        return -1;
    }
    Interrupt_Dispatch[num] = function;
    lock_release_writer(&idt_dispatch_table_lock);
    return 0;
}


void interrupts_init(void) {

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

    pic_init();

    struct idt_ptr idtp;
    idtp.limit = (sizeof(struct idt_entry) * IDT_Size) - 1;
    idtp.base = (uintptr_t)&IDT;
    idt_load(&idtp);
}


void common_interrupt_handler(struct cpu_state r) {
    assert(r.interrupt_number < IDT_Size);
    if (Interrupt_Dispatch[r.interrupt_number] != NULL) {
        Interrupt_Dispatch[r.interrupt_number](&r);
    } else if (r.interrupt_number < 32) {
        log(Log_Error, "Unhandled Exception Triggered!");
        logf(Log_Error,
             "\tds: %lx\n\t"
             "rdi: %lx\n\t"
             "rsi: %lx\n\t"
             "rbp: %lx\n\t"
             "rsp: %lx\n\t"
             "rbx: %lx\n\t"
             "rdx: %lx\n\t"
             "rcx: %lx\n\t"
             "rax: %lx\n\t"
             "interrupt_number: %lx\n\t"
             "err_code: %lx\n\t"
             "rip: %lx\n\t"
             "cs: %lx\n\t"
             "eflags: %lx\n\t"
             "rsp: %lx\n\t"
             "ss: %lx\n\t"
             "cr2: %p\n",
             r.ds, r.rdi, r.rsi, r.rbp, r.rsp, r.rbx, r.rdx, r.rcx, r.rax,
             r.interrupt_number, r.err_code, r.rip, r.cs, r.rflags, r.rsp,
             r.ss, read_cr2());

        panic();
    }
    pic_end_of_interrupt(r.interrupt_number);
}
