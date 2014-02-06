#include <stddef.h>
#include <stdint.h>
#include "gdt.c"
#include "idt.c"
#include "isr.c"
#include "terminal.h"


/* Check if the compiler thinks if we are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif
 
 
void kernel_main()
{
	gdt_install();
	idt_install();
	isrs_install();
	term_initialize();
	term_writestring("Interrupt?\n");
	int i = 5;
	i = i / 0;
	term_writestring("Hello, Kernel!");
}
