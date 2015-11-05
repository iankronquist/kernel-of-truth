#include "gdt.h"
/*GDT stands for "Global Descriptor Table". 
It is a table available on i386 (ix86?)
processors. The GDT is a list of 64 bit entries.
GRUB (or our boot) loader will install the GDT for us.
If we overwrite the GDT we will cause a 'triple fault'
and the machine will be reset.

Based on the tutorial, we will write a GDT with only 3 fields
and will not concern ourselves with the other fields.

The above was paraphrased from:
http://www.osdever.net/bkerndev/Docs/gdt.htm
*/

typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned int uint;

void gdt_set_gate(int num, unsigned long base, unsigned long limit,
                  uchar access, uchar granularity)
{
    //Set descriptor base access
    gdt[num].base_low = (base & 0xffff);
    gdt[num].base_middle = (base >> 16) & 0xff;
    gdt[num].base_high = (base >> 24) & 0xff;

    //Set descriptor limits
    gdt[num].limit_low = (limit & 0xffff);
    gdt[num].granularity = ((limit >> 16) & 0xf0);

    //Set granularity flags
    gdt[num].granularity |= (granularity & 0xf0);
    gdt[num].access = access;
}

void gdt_install()
{
    //Setup GDT pointer and limit
    gdtp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gdtp.base = &gdt;

    // NULL descriptor
    gdt_set_gate(0, 0, 0, 0, 0);

    /* The second entry is our Code Segment. The base address
	*  is 0, the limit is 4GBytes, it uses 4KByte granularity,
	*  uses 32-bit opcodes, and is a Code Segment descriptor.
	*  Please check the table above in the tutorial in order
	*  to see exactly what each value means */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* The third entry is our Data Segment. It's EXACTLY the
	*  same as our code segment, but the descriptor type in
	*  this entry's access byte says it's a Data Segment */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* Flush out the old GDT and install the new changes */
    gdt_flush();
}
