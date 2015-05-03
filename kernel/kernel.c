#include <stddef.h>
#include <stdint.h>
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "terminal.h"
#include "kabort.h"
#include "kassert.h"

 
void kernel_main()
{
	gdt_install();
	idt_install();
	isrs_install();
	term_initialize();
	term_writestring("Interrupt?");
	//kassert(0 == 0);
    int i = 0;
    int b = 128;
    int d = 1;
    kprint_int("\nisr0: ", (int)isr0);
    kprint_int("\nidt0_low: ", idt[0].base_lo);
    kprint_int("\nidt0_sel: ", idt[0].sel);
    kprint_int("\nidt0_always0: ", idt[0].always0);
    kprint_int("\nidt0_flags: ", idt[0].flags);
    kprint_int("\nidt0_base_hi: ", idt[0].base_hi);

    d = b/i;
    kprint_int("\nd: ", d);
}
