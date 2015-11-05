#ifndef GDT_H
#define GDT_H

#include <stdint.h>

extern void gdt_flush();

/* the __attribute__((packed))
tells the compiler not to 'optimize' the struct for us by 
packing.
*/
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

/* the following is a special pointer representing
the max bytes taken up by the GDT -1 */
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct gdt_entry gdt[3];
struct gdt_ptr gdtp;

/* A function in start.am. Used to properly 
reload the new segment registers */
//extern void gdt_flush();

void gdt_set_gate(uint32_t num, uint64_t base, uint64_t limit,
                  uint8_t access, uint8_t granularity);

void gdt_install();

#endif
