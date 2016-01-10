#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#include <arch/x86/paging.h>

#include <libk/kmem.h>

struct process {
    uint32_t id;
    uint32_t esp, ebp, eip, cr3, cs;
    struct process *next;
};

struct process *Cur_Proc;

void proc_init(void);
struct cpu_state *create_proc(void(*entrypoint)());
void preempt(void);

extern void enter_proc(uint32_t esp);
extern void set_up_stack(uint32_t new_stack, uint32_t new_eip);

uint32_t get_next_pid();

#endif
