#include <stddef.h>
#include <stdint.h>
#include "terminal.h"
#include "kabort.h"
#include "kassert.h"


/* Check if the compiler thinks if we are targeting the wrong operating system. */
//Useful for possible new contributers.
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif
 
 
void kernel_main()
{
	term_initialize();
	term_writestring("Hello, Kernel!");
}
