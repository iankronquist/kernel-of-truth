#pragma once

#include <truth/types.h>


// Halt CPU.
extern void halt(void);

// Gets the CPU time step counter value.
extern uint64_t cpu_time(void);

// Puts CPU into sleep state until awoken by interrupt.
void cpu_sleep_state(void);
