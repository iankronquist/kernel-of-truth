#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#include <arch/x86/paging.h>
#include <arch/x86/idt.h>
#include <drivers/timer.h>

#include <libk/kmem.h>

/* Represents a single process.
 * Processes are kept in a simple circularly linked list.
 *
 * @id: A single process id. Currently, process ids increase monotonically, and
 * will eventually loop around. However, do not rely on this behavior.
 * @user_esp: The stack used by user space programs.
 * @kernel_esp: The stack used by kernel space programs, including syscalls.
 * @cr3: The top level paging directory.
 * @next: The next process to be run.
 */
struct process {
    uint32_t id;
    uint32_t user_esp;
    uint32_t kernel_esp;
    uint32_t cr3;
    struct process *next;
};

/* Set up multi-processing.  */
void proc_init(void);

/* Create a new process.
 * The process will have all of its memory reserved
 * including its stacks and paging directories. It will not be scheduled.
 */
struct process *create_proc(void(*entrypoint)());

/* Yield processor time to the scheduler.  */
void preempt(void);

/* Schedule a process to be run. */
void schedule_proc(struct process *proc);

#endif
