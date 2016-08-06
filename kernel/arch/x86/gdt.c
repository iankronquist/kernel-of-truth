#include <string.h>

#include <arch/x86/gdt.h>

#include <truth/types.h>

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

    uint8_t segment_type:4;
    uint8_t descriptor_type:1;
    uint8_t descriptor_privilege:2;
    uint8_t present:1;

    uint8_t limit_high:4;
    uint8_t available:1;
    uint8_t bit64:1;
    uint8_t op_size:1;
    uint8_t granularity:1;

    uint8_t base_high;
} Gdt[3];

/* A special pointer representing a pointer to the <gdt> and its size.
 * For whatever reason, we only record the full size minus 1.
 */
static struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
}__attribute__((packed)) Gdtp;

#define SEGMENT_32_BIT 0
#define SEGMENT_64_BIT 1

#define SEGMENT_RW_DATA 2
#define SEGMENT_RX_CODE 10

#define KERNEL_MODE 0
#define USER_MODE 3

#define GRANULARITY_BYTE 0
#define GRANULARITY_PAGE 1

#define SYSTEM_TYPE 0
#define CODE_OR_DATA_TYPE 1

#define OP_16_bit 0
#define OP_32_bit 1

/* Properly initialize an entry in the <gdt> */
static void gdt_set_gate(uint32_t index, uint64_t base, uint64_t limit,
                         bool descriptor_type, uint8_t descriptor_privilege,
                         bool bit64, bool granularity, bool op_size,
                         uint8_t segment_type) {
    // Zero fields.
    memset(&Gdt[index], 0, sizeof(struct gdt_entry));

    //Set descriptor base access
    Gdt[index].base_low = (base & 0xffff);
    Gdt[index].base_middle = (base >> 16) & 0xff;
    Gdt[index].base_high = (base >> 24) & 0xff;

    //Set descriptor limits
    Gdt[index].limit_low = (limit & 0xffff);
    Gdt[index].limit_high = (limit >> 16) & 0x0f;

    Gdt[index].bit64 = bit64;
    Gdt[index].descriptor_type = descriptor_type;
    Gdt[index].descriptor_privilege = descriptor_privilege;
    Gdt[index].granularity = granularity;
    Gdt[index].op_size = op_size;
    Gdt[index].segment_type = segment_type;

    Gdt[index].present = 1;
}


/* An assembly function which loads the <gdt>.
 * It is a simple wrapper around the **lgdt** instruction.
 */
extern void gdt_flush(struct gdt_ptr gdtp);

void gdt_install() {
    //Setup GDT pointer and limit
    Gdtp.limit = sizeof(Gdt) - 1;
    Gdtp.base = (uint32_t)&Gdt;

    // NULL descriptor
    memset(&Gdt[0], 0, sizeof(struct gdt_entry));

    // The second entry is the Code Segment. The base address
    // is 0, the limit is 4GB, it uses 4KB granularity,
    //  uses 32-bit opcodes, and is a Code Segment descriptor.
    gdt_set_gate(1, 0, 0xffffffff, CODE_OR_DATA_TYPE, KERNEL_MODE,
            SEGMENT_32_BIT, GRANULARITY_PAGE, OP_32_bit, SEGMENT_RX_CODE);

    // The third entry is the Data Segment. It's exactly the
    // same as the code segment, but the descriptor type in
    // this entry's access byte says it's a Data Segment */
    gdt_set_gate(2, 0, 0xffffffff, CODE_OR_DATA_TYPE, KERNEL_MODE,
            SEGMENT_32_BIT, GRANULARITY_PAGE, OP_32_bit, SEGMENT_RW_DATA);

    gdt_flush(Gdtp);
}
