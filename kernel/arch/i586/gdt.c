#include "gdt.h"


struct gdt_record create_gdt_record(uint32_t base, uint32_t limit, 
    char access_byte, char flags)
{
    struct gdt_record gdt_entry;
    //limit has a maximum size of 2^20-1
    kassert(limit < 1048576);
    // base has a maximum size of 2^23
    kassert(base < 8388608);
    // flags has a maximum size of 2^3
    kassert(flags < 8);
    // the top bytes of access bytes must be 1
    kassert(access_byte & 255);
    /* bytes 0-15 of the GDT are the limit. 1048575 is a bitmask for the
       twenty high bytes. It equals 2^20-1 */
    gdt_entry.limit_first = 1048575 & limit;
    //The first 16 bytes of base are mapped to the second 16 (16-31) of the
    //gdt entry
    gdt_entry.base_first = base;
    // Put bits 16:19 of the limit in bits 51-48 of the gdt,
    // in other words the first 3 bits of the sixth eighth of the  gdt_record
    gdt_entry.base_mid = base & 16711680;
    // Set the access byte
    gdt_entry.access = access_byte;
    //Put the last part of the limit into bytes 16-19 of the gdt_entry
    gdt_entry.flags_and_limit_last |= limit >> 8;  
    // Put the three flag bits into buts 52:55 of the gdt, in other words,
    // into bytes 8-11 of the last quarter of the gdt
    gdt_entry.flags_and_limit_last |= flags << 8;
    // Put the top 8 bytes of the base into the last quarter of gdt_record
    gdt_entry.base_last |= (base & 65280) >> 8;
    return gdt_entry;
}

void set_up_gdt()
{
    
}
