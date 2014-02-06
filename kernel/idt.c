//Included for the memset function.
#include "idt.h"
#include "../tlibc/tmem/mem.h"
// This is modeled after the material on the following tutorial:
// http://www.osdever.net/bkerndev/Docs/idt.htm

typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, 
	unsigned char flags)
{
	idt[num].base_lo = base << 16;
	idt[num].base_hi = base >> 16;
	idt[num].sel = sel;
	idt[num].flags = flags;
}

void idt_install()
{
	// 256 is the number of entries in the table. 
	iidtp.limit = (sizeof(struct idt_entry) * 256) - 1;
	
	memset(&idt, 0, sizeof(struct idt_entry) * 256);
	
	idt_load();
}


