#include "kputs.c"
#include "kabort.c"
#include "idt.c"

extern void isr0;
extern void isr1;
extern void isr2;
extern void isr3;
extern void isr4;
extern void isr5;
extern void isr6;
extern void isr7;
extern void isr8;
extern void isr9;
extern void isr10;
extern void isr11;
extern void isr12;
extern void isr13;
extern void isr14;
extern void isr15;
extern void isr16;
extern void isr17;
extern void isr18;
extern void isr19;
extern void isr20;
extern void isr21;
extern void isr22;
extern void isr23;
extern void isr24;
extern void isr25;
extern void isr26;
extern void isr27;
extern void isr28;
extern void isr29;
extern void isr30;
extern void isr31;

void isrs_install()
{
	idt_set_gate(0,(unsigned)isr0, 0x08, 0x8e);
	idt_set_gate(1,(unsigned)isr1, 0x08, 0x8e);
	idt_set_gate(2, isr2, 0x08, 0x8e);
	idt_set_gate(3, isr3, 0x08, 0x8e);
	idt_set_gate(4, isr4, 0x08, 0x8e);
	idt_set_gate(5, isr5, 0x08, 0x8e);
	idt_set_gate(6, isr6, 0x08, 0x8e);
	idt_set_gate(7, isr7, 0x08, 0x8e);
	idt_set_gate(8, isr8, 0x08, 0x8e);
	idt_set_gate(9, isr9, 0x08, 0x8e);
	idt_set_gate(10, isr10, 0x08, 0x8e);
	idt_set_gate(11, isr11, 0x08, 0x8e);
	idt_set_gate(12, isr12, 0x08, 0x8e);
	idt_set_gate(13, isr13, 0x08, 0x8e);
	idt_set_gate(14, isr14, 0x08, 0x8e);
	idt_set_gate(15, isr15, 0x08, 0x8e);
	idt_set_gate(16, isr16, 0x08, 0x8e);
	idt_set_gate(17, isr17, 0x08, 0x8e);
	idt_set_gate(18, isr18, 0x08, 0x8e);
	idt_set_gate(19, isr19, 0x08, 0x8e);
	idt_set_gate(20, isr20, 0x08, 0x8e);
	idt_set_gate(21, isr21, 0x08, 0x8e);
	idt_set_gate(22, isr22, 0x08, 0x8e);
	idt_set_gate(23, isr23, 0x08, 0x8e);
	idt_set_gate(24, isr24, 0x08, 0x8e);
	idt_set_gate(25, isr25, 0x08, 0x8e);
	idt_set_gate(26, isr26, 0x08, 0x8e);
	idt_set_gate(27, isr27, 0x08, 0x8e);
	idt_set_gate(28, isr28, 0x08, 0x8e);
	idt_set_gate(29, isr29, 0x08, 0x8e);
	idt_set_gate(30, isr30, 0x08, 0x8e);
	idt_set_gate(31, isr31, 0x08, 0x8e);
}


/* This defines what the stack looks like after an ISR was running */
struct regs
{
    unsigned int gs, fs, es, ds;      /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    unsigned int int_no, err_code;    /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
};


void fault_handler(struct regs *r)
{
	/* Is this a fault whose number is from 0 to 31? */
	if (r->int_no < 32)
	{
		/* Display the description for the Exception that occurred.
		*  In this tutorial, we will simply halt the system using an
		*  infinite loop */
		kputs(exception_messages[r->int_no]);
		kputs(" Exception. System Halted!\n");
		kabort();
	}
	//This should never happen.
	kputs("ERROR UNEXPECTED INTERRUPT!\n");
	kabort();
}
