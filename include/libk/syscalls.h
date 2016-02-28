#ifndef SYSCALLS_H
#define SYSCALLS_H

#define SYSCALL_MAX 1

void syscalls_install();
extern void _syscalls_handler(void);
extern void kputs(char* str);

#endif
