#ifndef GDT_H
#define GDT_H
#include <stdint.h>
#include "../../kernel-functions/kassert.h"

/*  For a thorough explanation of gdts go here:
    http://wiki.osdev.org/Global_Descriptor_Table
*/

typedef uint64_t GDT_t;

struct gdt_location {
    size_t table_size;
    GDT_t* table_start;
}__attribute__((packed));

/* creates a gdt record with the specified settings */
GDT_t create_gdt_record(uint32_t base, uint32_t limit, uint8_t access_byte,
                           uint8_t flags);

/* sets up the kernel's gdt */
void set_up_gdt();

/* calls the lgdt and enter protected mode */
void enter_protected_mode();

struct gdt_location* gdt_ptr;

#endif
