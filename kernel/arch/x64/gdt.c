#include <truth/types.h>

/* The Global Descriptor Table and its entries.
 * x86 uses a backward, antiquated segmented memory model for compatibility
 * with computers which were around in the late Cretaceous. The Global
 * Descriptor Table, or Gdt, can control whether or not certain sections of
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
 * This structure represents a single entry in the Gdt. See figure 3-8,
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
};

struct gdt_entry64 {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
    uint32_t base_highest;
    uint32_t reserved0;
};

struct tss_entry {
    uint32_t reserved0;
    uint64_t stack0;     /* This is not naturally aligned, so packed is needed. */
    uint64_t stack1;
    uint64_t stack2;
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} pack;

struct tss_entry Tss = {
    .reserved0 = 0,
    .stack0 = 0,
    .stack1 = 0,
    .stack2 = 0,
    .reserved1 = 0,
    .ist = { 0, 0, 0, 0, 0, 0, 0},
    .reserved2 = 0,
    .reserved3 = 0,
    .iomap_base = 0,
};


#define GRAN_64_BIT_MODE (1 << 5)
#define GRAN_32_BIT_MODE (1 << 6)
#define GRAN_4KIB_BLOCKS (1 << 7)

#define GDT_ENTRY(base, limit, access, granularity) \
    { (limit) & 0xffff,                              /* limit_low */ \
      (uint16_t) ((base) >> 0 & 0xffff),             /* base_low */ \
      (uint8_t) ((base) >> 16 & 0xff),               /* base_middle */ \
      (access) & 0xff,                               /* access */ \
      ((limit) >> 16 & 0x0f) | ((granularity) & 0xf0),     /* granularity */ \
      (uint8_t) ((base) >> 24 & 0xff),                 /* base_high */ }

#define GDT_ENTRY64(base, limit, access, granularity) \
    { (limit) & 0xffff,                              /* limit_low */ \
      (uint16_t) ((base) >> 0 & 0xffff),             /* base_low */ \
      (uint8_t) ((base) >> 16 & 0xff),               /* base_middle */ \
      (access) & 0xff,                               /* access */ \
      ((limit) >> 16 & 0x0f) | ((granularity) & 0xf0),     /* granularity */ \
      (uint8_t) ((base) >> 24 & 0xff),                 /* base_high */ }, \
    { (uint16_t) ((base) >> 32 & 0xffff),            /* base_highest */ \
      (uint16_t) ((base) >> 48 & 0xffff),            /* base_highest */ \
      0,                                             /* reserved0 */ \
      0,                                             /* reserved0 */ \
      0,                                             /* reserved0 */ \
      0,                                               /* reserved0 */ }

struct gdt_entry Gdt[] = {
    // NULL segment
    GDT_ENTRY(0, 0, 0, 0),

    // Kernel Code Segment.
    GDT_ENTRY(0, 0xffffffff, 0x9a, GRAN_64_BIT_MODE | GRAN_4KIB_BLOCKS),

    // Kernel Data Segment.
    GDT_ENTRY(0, 0xffffffff, 0x92, GRAN_64_BIT_MODE | GRAN_4KIB_BLOCKS),

    // User Code Segment.
    GDT_ENTRY(0, 0xffffffff, 0xfa, GRAN_64_BIT_MODE | GRAN_4KIB_BLOCKS),

    // User Data Segment.
    GDT_ENTRY(0, 0xffffffff, 0xf2, GRAN_64_BIT_MODE | GRAN_4KIB_BLOCKS),

    // Task Switch Segment.
    GDT_ENTRY64(0ull, sizeof(Tss) - 1, 0xe9, 0x00),
};

uint16_t Gdt_Size = sizeof(Gdt) - 1;
