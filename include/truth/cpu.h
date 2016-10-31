#pragma once

#include <truth/types.h>

// The state of the CPU before an interrupt occurs.
struct cpu_state;

// Interrupt Service Routine function signature.
// ISRs with this signature are installed to a dispatch table.
typedef void (isr_f)(struct cpu_state *);

/* Install an interrupt handler.
 * The handler will have the interrupt number @num, and when triggered it will
 * execute @function. If @privileged is set to false, the interrupt will be
 * able to be raised by ring 3 code. If false, it will only be able to be
 * raised by ring 0 code. @return 0 if the interrupt is successfully installed
 * and -1 if that interrupt number has already been registered.
 */
int install_interrupt(uint8_t num, isr_f function);

void init_interrupts(void);

// Disable interrupts
extern void disable_interrupts(void);

// Enable interrupts
extern void enable_interrupts(void);

// Halt CPU
extern void halt(void);

// Get's the CPU time step counter value
extern uint64_t cpu_time(void);

// Get the current function base pointer
extern uintptr_t get_base_pointer(void);
