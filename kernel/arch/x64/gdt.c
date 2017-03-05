#include <truth/types.h>

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
    uint64_t stack0;
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

#define gdt_entry(base, limit, access, granularity) \
    { (limit) & 0xffff, \
      (uint16_t) ((base) >> 0  & 0xffff), \
      (uint8_t)  ((base) >> 16 & 0xff), \
      (access) & 0xff, \
      ((limit) >> 16 & 0x0f) | ((granularity) & 0xf0), \
      (uint8_t) ((base) >> 24 & 0xff), \
    }

#define gdt_entry64(base, limit, access, granularity) \
    { (limit) & 0xffff, \
      (uint16_t) ((base) >> 0  & 0xffff), \
      (uint8_t)  ((base) >> 16 & 0xff), \
      (access) & 0xff, \
      ((limit) >> 16 & 0x0f) | ((granularity) & 0xf0), \
      (uint8_t) ((base) >> 24 & 0xff), \
    }, \
    { (uint16_t) ((base) >> 32 & 0xffff), \
      (uint16_t) ((base) >> 48 & 0xffff), \
      0, \
      0, \
      0, \
      0, \
    } \

struct gdt_entry GDT[] = {
    // NULL segment
    gdt_entry(0, 0, 0, 0),

    // Kernel Code Segment.
    gdt_entry(0, 0xffffffff, 0x9a, GRAN_64_BIT_MODE | GRAN_4KIB_BLOCKS),

    // Kernel Data Segment.
    gdt_entry(0, 0xffffffff, 0x92, GRAN_64_BIT_MODE | GRAN_4KIB_BLOCKS),

    // User Code Segment.
    gdt_entry(0, 0xffffffff, 0xfa, GRAN_64_BIT_MODE | GRAN_4KIB_BLOCKS),

    // User Data Segment.
    gdt_entry(0, 0xffffffff, 0xf2, GRAN_64_BIT_MODE | GRAN_4KIB_BLOCKS),

    // Task Switch Segment.
    gdt_entry64(0ull, sizeof(Tss) - 1, 0xe9, 0x00),
};

uint16_t GDT_Size = sizeof(GDT) - 1;
