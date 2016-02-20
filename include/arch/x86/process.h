#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#include <arch/x86/paging.h>
#include <arch/x86/idt.h>
#include <drivers/timer.h>

#include <libk/kmem.h>

struct process {
    uint32_t id;
    uint32_t user_esp;
    uint32_t kernel_esp;
    uint32_t cr3;
    struct process *next;
};

void proc_init();
struct process *create_proc(void(*entrypoint)());
void preempt();
void schedule_proc(struct process *proc);
uint32_t get_next_pid();

extern uint32_t get_flags(void);
extern uint32_t get_page_dir(void);
extern void _process_handler(void);
extern void switch_task(uint32_t esp, uint32_t cr3, uint32_t *kernel_esp);


#endif
