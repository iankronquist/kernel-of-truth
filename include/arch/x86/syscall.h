#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdarg.h>

#include <arch/x86/idt.h>
#include <libk/klog.h>

extern void klog(char *message);
extern void klogf(char* string, ...);

void install_syscall();

extern void _syscall_handler(void);

#endif
