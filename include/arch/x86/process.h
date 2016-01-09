#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#include <arch/x86/paging.h>

#include <libk/kmem.h>

struct cpu_state {
    // pushad
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    // pushf
    uint32_t eflags;
    // mov eax, eip
    // push eax
    uint32_t eip;
    // mov eax, cr3
    // push eax
    uint32_t* cr3;
    // mov eax, cs
    // push eax
    uint32_t cs;
};

struct process {
    uint32_t id;
    struct cpu_state state;
    struct process *next;
};

struct process *Cur_Proc;

void proc_init(void);
struct cpu_state *create_proc(void(*entrypoint)());
void preempt(void);

extern void switch_proc(struct cpu_state *state);
extern void get_current_regs(struct cpu_state *state);
extern void set_eip(struct cpu_state *state);

uint32_t get_next_pid();

#endif
