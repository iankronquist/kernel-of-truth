#include <truth/interrupts.h>
#include <truth/lock.h>
#include <truth/panic.h>
#include <arch/x64/idt.h>
#include <arch/x64/interrupts.h>
#include <arch/x64/pic.h>
#include <arch/x64/control_registers.h>

#define Interrupt_Handlers_Count 10
static struct lock dispatch_table_lock = Lock_Clear;

static interrupt_handler_f *Interrupt_Dispatch[IDT_Size][Interrupt_Handlers_Count] = {{0}};


void interrupts_dispatcher(struct interrupt_cpu_state r) {
    assert(r.interrupt_number < IDT_Size);
    bool handled = false;
    for (size_t i = 0; i < Interrupt_Handlers_Count; ++i) {
        if (Interrupt_Dispatch[r.interrupt_number][i] != NULL) {
            Interrupt_Dispatch[r.interrupt_number][i](&r);
            handled = true;
        }
    }
    if (r.interrupt_number < 32 && handled) {
        log(Log_Error, "Unhandled Exception Triggered!");
        logf(Log_Error,
             "\tds: %lx\n\t"
             "r15: %lx\n\t"
             "r14: %lx\n\t"
             "r13: %lx\n\t"
             "r12: %lx\n\t"
             "r11: %lx\n\t"
             "r10: %lx\n\t"
             "r9: %lx\n\t"
             "r8: %lx\n\t"
             "rdi: %lx\n\t"
             "rsi: %lx\n\t"
             "rbp: %lx\n\t"
             "rdx: %lx\n\t"
             "rcx: %lx\n\t"
             "rbx: %lx\n\t"
             "rax: %lx\n\t"
             "interrupt_number: %lx\n\t"
             "err_code: %lx\n\t"
             "rip: %lx\n\t"
             "cs: %lx\n\t"
             "rflags: %lx\n\t"
             "rsp: %lx\n\t"
             "ss: %lx\n\t"
             "cr2: %p\n",
             r.ds, r.r15, r.r14, r.r13, r.r12, r.r11, r.r10, r.r9, r.r8,
             r.rdi, r.rsi, r.rbp, r.rdx, r.rcx, r.rbx, r.rax,
             r.interrupt_number, r.err_code, r.rip, r.cs, r.rflags, r.rsp,
             r.ss, read_cr2());

        panic();
    }
    pic_end_of_interrupt(r.interrupt_number);
}


void interrupts_init(void) {
    idt_init();
    pic_init();
    interrupts_enable();
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
            Interrupt_Dispatch[interrupt_number][i] = handler;
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
        if (Interrupt_Dispatch[interrupt_number][i] == handler) {
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
