#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdarg.h>

#include <arch/x86/idt.h>
#include <libk/klog.h>

extern void klog(char *message);
extern void kputs(char* string);
extern void spawn(void (*entrypoint)(void));
extern void exec(void (*entrypoint)(void));
extern void exit();

void install_syscall();
void bad_syscall(uint32_t bad_syscall_num);

extern void _syscall_handler(void);

#endif
