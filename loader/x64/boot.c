#include <truth/types.h>

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

struct tss_entry Boot_TSS = {
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

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
};

#define Gran_64_Bit_Mode (1 << 5)
#define Gran_32_Bit_Mode (1 << 6)
#define Gran_4_KB_BLocks (1 << 7)

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

struct gdt_entry Boot_GDT[] = {
    // NULL segment
    gdt_entry(0, 0, 0, 0),

    // Kernel Code Segment.
    gdt_entry(0, 0xffffffff, 0x9a, Gran_64_Bit_Mode | Gran_4_KB_BLocks),

    // Kernel Data Segment.
    gdt_entry(0, 0xffffffff, 0x92, Gran_64_Bit_Mode | Gran_4_KB_BLocks),

    // User code and data segments would go here, but we don't need them in
    // early boot.
    // Still put dummy values here so we can reuse the same TSS segment
    // number as the kernel proper.
    {0},

    {0},

    // Task Switch Segment.
    gdt_entry64(0ull, sizeof(Boot_TSS) - 1, 0xe9, 0x00),
};

struct gdt_register {
    uint16_t limit;
    uint64_t base;
} pack;

struct gdt_register Boot_GDT_Register = {
    .limit = sizeof(Boot_GDT) - 1,
    .base = (uint64_t)&Boot_GDT,
};


void loader_main(uint64_t multiboot_magic, uint64_t multiboot_info_physical) {
}
