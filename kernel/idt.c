//Included for the memset function.
#include "../tlibc/tmem/mem.c"
// This is modeled after the material on the following tutorial:
// http://www.osdever.net/bkerndev/Docs/idt.htm

typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

// This macro is implemented in assmbly and can be fount in idt.s
extern void idt_load();

// Define an entry in the IDT 
struct idt_entry
{
	ushort base_lo;
	ushort sel; // Kernel segment goes here.
	ushort always0; 
	uchar flags; //Set using the table.
	ushort base_hi;
}__attribute((packed));

struct idt_ptr
{
	ushort limit;
	uint base;
}__attribute((packed));

// http://www.osdever.net/bkerndev/Docs/idt.htm
/* Declare an IDT of 256 entries. I only plan to use 32 of them,
*  but this is to prevent unhandeled interrupt exceptions.
*  If any undefined IDT entry is hit, it normally
*  will cause an "Unhandled Interrupt" exception. Any descriptor
*  for which the 'presence' bit is cleared (0) will generate an
*  "Unhandled Interrupt" exception */
struct idt_entry idt[256];
struct idt_ptr iidtp;


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


