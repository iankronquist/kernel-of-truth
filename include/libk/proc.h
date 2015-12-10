#ifndef PROC_H
#define PROC_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


#ifdef ARCH_x86
#include <arch/x86/cpu_state.h>
#endif

#ifdef ARCH_USERLAND
// There is no reason to have this differ from the original x86 version. It
// just contains the struct definition.
#include <arch/x86/cpu_state.h>
#endif

#define PROC_SPECIAL 0

typedef uint32_t true_pid_t;

struct process {
    true_pid_t id;
    uint32_t priority;
    struct cpu_state state;
    // This may come in handy soon...
    // bool kernel_mode;

    // Private! Do not touch this variable, this may be replaced when the
    // process table is replaced with a hash table.
    struct process *next;
};

// Later on I will change the entry point to a pointer to an elf executable,
// but that requires a loader.
int start_proc(void (*entry_point)(void));
int resume_proc(true_pid_t pid);
int finish_proc(true_pid_t pid);

#endif
