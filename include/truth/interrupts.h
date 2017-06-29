#pragma once

#include <truth/types.h>

struct interrupt_cpu_state;

typedef bool (interrupt_handler_f)(struct interrupt_cpu_state *);

void interrupts_init(void);
void interrupts_fini(void);
enum status interrupt_register_handler(unsigned int interrupt_number,
                                       interrupt_handler_f handler);
enum status interrupt_unregister_handler(unsigned int interrupt_number,
                                         interrupt_handler_f handler);
void interrupts_enable(void);
void interrupts_disable(void);
