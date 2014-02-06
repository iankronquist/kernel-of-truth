#include "kputs.c"
#include "kabort.c"
#include "idt.h"

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

void isrs_install();

char *exception_messages[] =
{
	"Division By Zero Exception",
	"Debug Exception",
	"Non Maskable Interrupt Exception",
	"Breakpoint Exception",
	"Into Detected Overflow Exception",
	"Out of Bounds Exception",
	"Invalid Opcode Exception",	
	"No Coprocessor Exception",	
	"Double Fault Exception",	
	"Coprocessor Segment Overrun Exception",
 	"Bad TSS Exception",
 	"Segment Not Present Exception",
 	"Stack Fault Exception",
 	"General Protection Fault Exception",   
 	"Page Fault Exception",
 	"Unknown Interrupt Exception",
 	"Coprocessor Fault Exception",	
 	"Alignment Check Exception (486+)",     
 	"Machine Check Exception (Pentium/586+)",
	"Reserved Exceptions",
	"Reserved Exceptions",
	"Reserved Exceptions",
	"Reserved Exceptions",
	"Reserved Exceptions",
	"Reserved Exceptions",
	"Reserved Exceptions",
	"Reserved Exceptions",
	"Reserved Exceptions",
	"Reserved Exceptions",
	"Reserved Exceptions",
	"Reserved Exceptions",
	"Reserved Exceptions",
};


/* This defines what the stack looks like after an ISR was running */
struct regs
{
    unsigned int gs, fs, es, ds;      /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    unsigned int int_no, err_code;    /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
};


void fault_handler(struct regs *r);
