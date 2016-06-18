#ifndef IDT_H
#define IDT_H

#include <string.h>
#include <stddef.h>
#include <stdint.h>

#include <libk/kputs.h>
#include <libk/kabort.h>

#include <drivers/timer.h>

/* The state of the CPU when an interrupt is triggered.
*/
struct regs {
    uint32_t ds; /* pushed the segs last */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* pushed by 'pusha' */
    uint32_t int_no, err_code; /* our 'push byte #' and ecodes do this */
    uint32_t eip, cs, eflags, useresp, ss; /* pushed by the processor automatically */
};

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

void idt_install();

#endif
