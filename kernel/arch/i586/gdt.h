#ifndef GDT_H
#define GDT_H
#include <stdint.h>
#include "../../kernel-functions/kassert.h"

/*  For a thorough explanation of gdts go here:
    http://wiki.osdev.org/Global_Descriptor_Table
*/


/*
    This struct represents a single entry in the GDT.
    Each record is eight bytes long.
*/
struct gdt_record {
    int16_t limit_first;
    int16_t base_first;
    int8_t base_mid;
    int8_t access;
    int8_t flags_and_limit_last;
    int8_t base_last;
    
}__attribute__((packed));


/*
    This struct represents the information to be passed to the lgdt
    instruction. It requires a pointer to the GDT and the size of the
    table.
    The size is actually the side of the table - 1, and has a max value of
    65535 (so the GDT has a max length of 65536).
    The compiler directive __attribute__((packed)); tells the compiler
    to position these values contiguously in memory as opposed to byte
    aligning them as is typical
*/
struct gdt_location {
    size_t table_size;
    struct gdt_record* table_start;
}__attribute__((packed));

/* creates a gdt record with the specified settings */
struct gdt_record create_gdt_record(uint32_t base, uint32_t limit, 
    char access_byte, char flags);
/* sets up the kernel's gdt */
void set_up_gdt();

/* calls the lgdt and enter protected mode */
//TODO decide on how to do this
void enter_protected_mode(struct gdt_location gdt);


#endif
