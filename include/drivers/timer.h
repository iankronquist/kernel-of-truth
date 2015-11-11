#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>
#include <arch/x86/io.h>
#include <libk/kputs.h>

#define TIMER_INTERRUPT_MASK (~1)
// Comes from hardware clock cycle of 1.19 MHz
#define TIMER_MAGIC_NUMBER 1193180

// Mapped to IRQ 0
#define TIMER_CHAN_0 0x40
// System specific timer
#define TIMER_CHAN_1 0x41
// Connected to system speaker
#define TIMER_CHAN_2 0x42
#define TIMER_COMMAND_REG 0x43

// See this resource for motivation on this constant
// http://www.osdever.net/bkerndev/Docs/pit.htm
#define RW_ONESHOT_SQUARE 0x36

extern void timer_handler();
void timer_irq_handler();
void timer_install();

#endif
