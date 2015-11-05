#ifndef GDT_H
#define GDT_H
/*GDT stands for "Global Descriptor Table". 
It is a table available on i386 (ix86?)
processors. The GDT is a list of 64 bit entries.
GRUB (or our boot) loader will install the GDT for us.
If we overwrite the GDT we will cause a 'triple fault'
and the machine will be reset.

Based on the tutorial, we will write a GDT with only 3 fields
and will not concern ourselves with the other fields.

The above was paraphrased from:
http://www.osdever.net/bkerndev/Docs/gdt.htm
*/

typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned int uint;

extern void gdt_flush();
/* the __attribute__((packed))
tells the compiler not to 'optimize' the struct for us by 
packing.
*/
struct gdt_entry {
    ushort limit_low;
    ushort base_low;
    uchar base_middle;
    uchar access;
    uchar granularity;
    uchar base_high;
} __attribute__((packed));

/* the following is a special pointer representing
the max bytes taken up by the GDT -1 */
struct gdt_ptr {
    ushort limit;
    uint base;
} __attribute__((packed));

struct gdt_entry gdt[3];
struct gdt_ptr gdtp;
/* A function in start.am. Used to properly 
reload the new segment registers */
//extern void gdt_flush();

void gdt_set_gate(int num, unsigned long base, unsigned long limit,
                  uchar access, uchar granularity);

void gdt_install();

#endif
