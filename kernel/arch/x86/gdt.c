#include <arch/x86/gdt.h>

void gdt_set_gate(uint32_t num, uint64_t base, uint64_t limit,
                  uint8_t access, uint8_t granularity)
{
    //Set descriptor base access
    gdt[num].base_low = (base & 0xffff);
    gdt[num].base_middle = (base >> 16) & 0xff;
    gdt[num].base_high = (base >> 24) & 0xff;

    //Set descriptor limits
    gdt[num].limit_low = (limit & 0xffff);
    gdt[num].granularity = ((limit >> 16) & 0x0f);

    //Set granularity flags
    gdt[num].granularity |= (granularity & 0xf0);
    gdt[num].access = access;
}

void gdt_install()
{
    //Setup GDT pointer and limit
    gdtp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gdtp.base = (uint32_t) & gdt;

    // NULL descriptor
    gdt_set_gate(0, 0, 0, 0, 0);

    /* The second entry is the Code Segment. The base address
    *  is 0, the limit is 4GB, it uses 4KB granularity,
    *  uses 32-bit opcodes, and is a Code Segment descriptor. */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* The third entry is the Data Segment. It's exactly the
    *  same as the code segment, but the descriptor type in
    *  this entry's access byte says it's a Data Segment */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* Flush out the old GDT and install the new changes */
    gdt_flush();
}
