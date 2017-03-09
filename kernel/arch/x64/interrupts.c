#include <truth/interrupts.h>
#include <arch/x86/idt.h>
#include <arch/x86/interrupt.h>
#include <arch/x86/pic.h>

#define Interrupt_Handlers_Count 10
static struct lock dispatch_table_lock = Lock_Clear;

static isr_f *Interrupt_Dispatch[IDT_Size][Interrupt_Handlers_Count] = {{0}};


void interrupts_dispatcher(struct cpu_state r) {
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


void interrupts_init(void) {
    idt_init();
    pic_init();
}


void interrupts_fini(void) {
    interrupts_disable();
    pic_fini();
}


enum status interrupt_register_handler(int interrupt_number, interrupt_handler_f handler) {
    enum status status;
    lock_acquire_writer(&dispatch_table_lock);
    for (size_t i = 0; i < Interrupt_Handlers_Count; ++i) {
        if (Interrupt_Dispatch[interrupt_number][i] == NULL) {
            Interrupt_Dispatch[interrupt_number][i] = function;
            status = Ok;
            goto out;
        }
    }
    status = Error_Count;
out:
    lock_release_writer(&dispatch_table_lock);
    return status;
}


enum status interrupt_unregister_handler(int interrupt_number, interrupt_handler_f handler) {
    enum status status;
    lock_acquire_writer(&dispatch_table_lock);
    for (size_t i = 0; i < Interrupt_Handlers_Count; ++i) {
        if (Interrupt_Dispatch[interrupt_number][i] == function) {
            Interrupt_Dispatch[interrupt_number][i] = NULL;
            status = Ok;
            goto out;
        }
    }
    status = Error_Absent;
out:
    lock_release_writer(&dispatch_table_lock);
    return status;

}
