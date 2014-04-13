#include "gdt.h"

struct gdt_record create_gdt_record(uint32_t base, uint32_t limit, 
    char access_byte, char flags)
{
    struct gdt_record gdt_entry;
    //limit has a maximum size of 2^20-1
    kassert(limit < 1048576);o
    // base has a maximum size of 2^23
    kassert(base < 8388608);
    // flags has a maximum size of 2^3
    kassert(flags < 8);
    /* bytes 0-15 of the GDT are the limit. 1048575 is a bitmask for the
       twenty high bytes. It equals 2^20-1 */
    gdt_entry.limit_first = 1048575 & limit;
    //The first 16 bytes of base are mapped to the second 16 (16-31) of the
    //gdt entry
    gdt_entry.base_first = base;
    /* bytes 16-19 of the limit are mapped to bytes 48-51 of the gdt
       4293918720 is the bitmask for the top 12 bytes. It equals 2^32 - 2^20
    */
    /* Set the access byte */
    //TODO implement the access byte
    gdt_entry.limit_last_and_flags_and_base_last = 0;  
    // Put bits 16:19 of the limit in bits 51-48 of the gdt,
    // in other words the first 3 bits of the last quarter of the  gdt_record
    gdt_entry.limit_last_and_flags_and_base_last |= limit >> 8;  
    // Put the three flag bits into buts 52:55 of the gdt, in other words,
    // into bytes 8-11 of the last quarter of the gdt
    gdt_entry.limit_last_and_flags_and_base_last |= flags << 7;
    // Put the top 7 bytes of the base into the last 7 of the gdt_record
    gdt_entry.limit_last_and_flags_and_base_last |= (base & 65024)<< 8;
}

#endif
