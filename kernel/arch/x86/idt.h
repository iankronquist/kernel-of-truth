#ifndef IDT_H
#define IDT_H
//Included for the memset function.
#include "../../../tlibc/string/string.h"
#include <stddef.h>
#include <stdint.h>

//extern void idt_load();

// Define an entry in the IDT 
struct idt_entry {
    uint16_t base_lo;
    uint16_t sel; // Kernel segment goes here.
    uint8_t always0; 
    uint8_t flags; //Set using the table.
    uint16_t base_hi;
}__attribute((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
}__attribute((packed));

struct regs {
    uint32_t ds;      /* pushed the segs last */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    uint32_t int_no, err_code;    /* our 'push byte #' and ecodes do this */
    uint32_t eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
};


extern void idt_load(uint32_t);
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel,
    uint8_t flags);

void idt_install();

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr32();

#endif
