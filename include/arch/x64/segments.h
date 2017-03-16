#pragma once

#define Segment_Null        0x00
#define Segment_Kernel_Code 0x08
#define Segment_Kernel_Data 0x10
#define Segment_User_Code   0x18
#define Segment_User_Data   0x20
#define Segment_TSS         0x28
#define Segment_RPL         0x03

#ifdef __C__

void tss_set_stack(void *stack);

#endif
