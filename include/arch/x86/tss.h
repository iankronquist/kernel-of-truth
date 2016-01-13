#ifndef TSS_H
#define TSS_H

#include <stdint.h>

// See figure 7-2, 32-Bit Task-State-Segment
// Volume 3-A
struct tss {
    uint16_t prev_task;
    uint16_t reserved0;
    uint32_t esp0;
    uint16_t ss0, reserved1;
    uint32_t esp1;
    uint16_t ss1, reserved2;
    uint32_t esp2;
    uint16_t ss2, reserved3;
    uint32_t cr3, eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint16_t es, reserved4, cs, reserved5, ss, reserved6, ds, reserved7, fs,
             reserved8, gs, reserved9;
    uint16_t ldt_segment_selector, reserved10;
    uint16_t base_addr;
};


#define TASK_BUSY 11
#define TASK_NOT_BUSY 9

#define TASK_PRESENT (1 << 15)

struct tss_descriptor {
    uint64_t segment_limit:16;
    uint64_t base_low:16;
    uint64_t base_mid:8;
    uint64_t type:4;
    uint64_t zero:1;
    uint64_t descriptor_privilege_level:2;
    uint64_t present:1;
    uint64_t limit:4;
    uint64_t available:1;
    uint64_t more_zeroes:2;
    uint64_t granularity:1;
    uint64_t base_high:8;
};

void set_up_tss();
extern void tss_flush(void);

#endif
