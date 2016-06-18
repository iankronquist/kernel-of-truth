#ifndef GDT_H
#define GDT_H

#include <stdint.h>

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
struct gdt_entry {
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
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
}__attribute__((packed)) gdtp;

/* Properly initialize an entry in the <gdt> */
void gdt_set_gate(uint32_t num, uint64_t base, uint64_t limit,
                  uint8_t access, uint8_t granularity);

/* An assembly function which loads the <gdt>.
 * It is a simple wrapper around the **lgdt** instruction.
 */
extern void gdt_flush(void);

/* Install the <gdt> and set it up as described. */
void gdt_install(void);

#endif
