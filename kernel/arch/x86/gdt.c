#include <arch/x86/gdt.h>

/* The Global Descriptor Table and its entries.
 * x86 uses a backward, antiquated segmented memory model for compatibility
 * with computers which were around in the late Cretaceous. The Global
 * Descriptor Table, or <gdt>, can control whether or not certain sections of
 * memory can run with elevated privileges (ring 0), as well as whether they
 * are used for code or data. Additionally, one 'gate', or entry in the GDT
 * called the Task Struct Segment or TSS, can be used to set up hardware
 * multitasking. We follow the example of professional OSs like Linux as well
 * as hobbyist systems like ToaruOS and do our best to ignore both the GDT and
 * hardware multitasking as much as possible. Better mechanisms exist for
 * enforcing access control, such as paging. Additionally, I read that properly
 * implemented software multitasking is faster than hardware multitasking
 * anyway. Nonetheless, we must placate the Gods of x86 and create a GDT and a
 * TSS.
 *
 * Our GDT will have 6 entries:
 *
 * 1. An empty entry because the manual says so.
 * 2. A kernel mode code segment.
 * 3. A kernel mode data segment.
 * 4. A user mode code segment (not yet created).
 * 5. A user mode data segment (not yet created).
 * 6. A TSS entry (not yet created).
 *
 * This structure represents a single entry in the <gdt>. See figure 3-8,
 * Chapter 3, Volume 3 of the Intel Manual and the rest of the chapter for more
 * information.
 */
static struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
}__attribute__((packed)) gdt[3];

/* A special pointer representing a pointer to the <gdt> and its size.
 * For whatever reason, we only record the full size minus 1.
 */
static struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
}__attribute__((packed)) gdtp;

/* An assembly function which loads the <gdt>.
 * It is a simple wrapper around the **lgdt** instruction.
 */
extern void gdt_flush(struct gdt_ptr gdtp);

/* Properly initialize an entry in the <gdt> */
static void gdt_set_gate(uint32_t num, uint64_t base, uint64_t limit,
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
    gdt_flush(gdtp);
}
