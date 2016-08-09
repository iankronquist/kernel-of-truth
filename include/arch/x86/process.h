#pragma once

#include <truth/types.h>
#include <truth/interrupts.h>

#include <arch/x86/paging.h>
#include <drivers/timer.h>

#include <truth/kmem.h>

struct virt_region;

// FIXME: Move to private architecture specific file.
/* All of the memory structures used by a single process.
 * This includes a virtual memory space cache, the kernel stack,
 * the user stack, and the page tables.
*/
struct proc_mem {
    page_frame_t page_dir;
    void *kernel_stack;
    void *user_stack;
    struct virt_region *free_virt;
};

/* Represents a single process.
 * Processes are kept in a simple circularly linked list.
 *
 * @id: A single process id. Currently, process ids increase monotonically, and
 * will eventually loop around. However, do not rely on this behavior.
 * @memory: All data required to describe and manage the process' memory.
 * @next: The next process to be run.
 */
struct process {
    struct proc_mem memory;
    struct process *next;
    uint32_t id;
};

/* Set up multi-processing.  */
void init_multitasking(void);

/* Create a new process.
 * The process will have all of its memory reserved
 * including its stacks and paging directories. It will not be scheduled.
 */
struct process *create_proc(void(*entrypoint)());

/* Yield processor time to the scheduler.  */
void preempt(void);

/* Schedule a process to be run. */
void schedule_proc(struct process *proc);

/* Get a pointer to the currently running process */
struct process *get_current_proc(void);
