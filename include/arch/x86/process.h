#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#include <arch/x86/paging.h>
#include <arch/x86/idt.h>
#include <drivers/timer.h>

#include <libk/kmem.h>

struct registers {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
};

struct process {
    uint32_t id;
    struct registers regs;
    struct process *next;
};

struct process *Cur_Proc;

void proc_init();
struct process *create_proc(void(*entrypoint)());
void preempt();
void schedule_proc(struct process *proc);

extern uint32_t get_flags(void);
extern uint32_t get_page_dir(void);
extern void _process_handler(void);
extern void switch_task(struct registers *old, struct registers *new);

uint32_t get_next_pid();

#endif
