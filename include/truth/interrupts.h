#pragma once

#include <truth/types.h>
#include <arch/x64/interrupt_numbers.h>

struct interrupt_cpu_state;

typedef bool (interrupt_handler_f)(struct interrupt_cpu_state *);

void interrupts_init(void);
void interrupts_fini(void);
enum status interrupt_register_handler(enum interrupt_number interrupt_number,
                                       interrupt_handler_f handler);
enum status interrupt_unregister_handler(enum interrupt_number interrupt_number,
                                         interrupt_handler_f handler);
void interrupts_end_interrupt(enum interrupt_number interrupt_number);
void interrupts_enable(void);
void interrupts_disable(void);
