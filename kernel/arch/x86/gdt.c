#include <arch/x86/gdt.h>
#include <libk/klog.h>

struct tss Tss;
struct tss_descriptor Tss_descriptor;

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
    gdtp.limit = (sizeof(struct gdt_entry) * 6) - 1;
    gdtp.base = (uint32_t) & gdt;

    // NULL descriptor
    gdt_set_gate(0, 0, 0, 0, 0);

    /* The second entry is our Code Segment. The base address
    *  is 0, the limit is 4GBytes, it uses 4KByte granularity,
    *  uses 32-bit opcodes, and is a Code Segment descriptor. */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* The third entry is our Data Segment. It's EXACTLY the
    *  same as our code segment, but the descriptor type in
    *  this entry's access byte says it's a Data Segment */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // Userspace code segment
    // Access flags: 0xfa Top nybble indicates it is present, ring 3, and
    // has a 1 at the end because the manual says so. The bottom nybble
    // indicates that it is executable and read/write.
    // Granularity flags: 0xcf. Only the top nybble is used. Use 4kb blocks for
    // the granularity, and set 32 bit protected mode.
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xfa, 0xcf);
    // Userspace data
    // Similar to the code segment except it's not executable.
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xf2, 0xcf);

    set_up_tss();

    // Flush out the old GDT and install the new changes
    gdt_flush();

    tss_flush();
}

void set_up_tss() {
    memset(&Tss, 0, sizeof(struct tss));
    Tss.esp0 = 0xbadc0de;
    Tss.ss0 = 0x10;
    uint32_t base = (uint32_t)&Tss;
    uint32_t limit = base + sizeof(struct tss);
    // Stupid TSS entry I don't actually use
    gdt_set_gate(5, base, limit, 0xe9, 0x00);
}

void set_tss_stack(uint32_t stack) {
    Tss.esp0 = stack;
}
