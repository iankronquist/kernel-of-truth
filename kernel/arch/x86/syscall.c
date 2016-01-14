#include <arch/x86/syscall.h>

void install_syscall() {
    idt_set_gate(0x80, (uint32_t)_syscall_handler, 0x08, 0xee);
}
