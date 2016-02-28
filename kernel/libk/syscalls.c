#include <libk/syscalls.h>
#include <arch/x86/idt.h>
#include <libk/kassert.h>
#include <libk/klog.h>

void syscalls_install() {
    idt_set_gate(0x80, (uint32_t)_syscalls_handler, 0x08, 0xee);
}

typedef void* (*syscall_func)();
static syscall_func syscalls[] = {
    (syscall_func)klog
};

void *syscall_handler(struct regs r) {
    kassert(r.eax < SYSCALL_MAX);
    syscall_func f = syscalls[r.eax];
    sys_kputs("test");
    void *ret = f(r.ebx, r.ecx, r.edx);
    return ret;
}
