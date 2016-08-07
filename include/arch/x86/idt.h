#pragma once

#include <arch/x86/cpu.h>

#include <truth/interrupts.h>

/* Disable interrupts */
extern void disable_interrupts(void);

/* Enable interrupts */
extern void enable_interrupts(void);
