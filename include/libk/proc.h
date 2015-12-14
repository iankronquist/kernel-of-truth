#ifndef PROC_H
#define PROC_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <libk/kmem.h>
#include <libk/process_table.h>

// FIXME: make all of this architecture independent!
#include <arch/x86/idt.h>
#include <drivers/timer.h>
#include <arch/x86/cpu_state.h>
extern void _resume_proc(uint32_t new_eip, uint32_t new_ebp, uint32_t new_esp);

#define PROC_SPECIAL 0

// 30 ms in between wakeups
#define QUANTUM 30



typedef uint32_t true_pid_t;

struct process {
    true_pid_t id;
    uint32_t priority;
    struct regs state;

    uint32_t *directory;
    // This may come in handy soon...
    // bool kernel_mode;

    // Private! Do not touch this variable, this may be replaced when the
    // process table is replaced with a hash table.
    struct process *next;
};

// Later on I will change the entry point to a pointer to an elf executable,
// but that requires a loader.
void start_proc(void (*entry_point)(void));
void resume_proc(struct process *resume);
void finish_proc(struct process *resume);
void init_scheduler(uint32_t *kernel_page_dir);

true_pid_t get_next_pid();


#endif
