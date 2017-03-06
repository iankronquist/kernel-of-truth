#include <arch/x64/port.h>

#define PIC1_Control 0x20
#define PIC1_Data (PIC1_Control + 1)
#define PIC1_Offset 0x20
#define PIC1_Next_PIC (PIC2_Identity << 1)
#define PIC2_Control 0xa0
#define PIC2_Data (PIC2_Control + 1)
#define PIC2_Offset 0x28
#define PIC2_Identity 0x02

#define PIC_End_Of_Interrupt 0x20

#define PIC2_First_Interrupt 0x08

#define ICW1_Init 0x10
#define ICW1_ICW4_Disable 0x01
#define ICW4_8086_Mode 0x01

#define PIC_All_IRQ_Mask 0xff


void pic_end_of_interrupt(uint8_t interrupt_number) {
    if (interrupt_number > PIC2_First_Interrupt) {
        write_port(PIC_End_Of_Interrupt, PIC2_Control);
    }
    write_port(PIC_End_Of_Interrupt, PIC1_Control);
}


void pic_init(void) {
    write_port(ICW1_Init | ICW1_ICW4_Disable, PIC1_Control);
    write_port(ICW1_Init | ICW1_ICW4_Disable, PIC2_Control);

    write_port(PIC1_Offset, PIC1_Data);
    write_port(PIC2_Offset, PIC2_Data);

    write_port(PIC1_Next_PIC, PIC1_Data);
    write_port(PIC2_Identity, PIC2_Data);

    write_port(ICW4_8086_Mode, PIC1_Data);
    write_port(ICW4_8086_Mode, PIC2_Data);

    write_port(PIC_All_IRQ_Mask, PIC1_Data);
    write_port(PIC_All_IRQ_Mask, PIC2_Data);
}


void pic_enable(uint8_t interrupt_number) {
    uint8_t pic;
    if (interrupt_number < 8) {
        pic = PIC1_Data;
    } else {
        pic = PIC2_Data;
        interrupt_number -= 8;
    }
    uint8_t interrupt_mask = read_port(pic);
    interrupt_mask |= 1 << interrupt_number;
    write_port(interrupt_mask, pic);
}


void pic_disable(uint8_t interrupt_number) {
    uint8_t pic;
    if (interrupt_number < 8) {
        pic = PIC1_Data;
    } else {
        pic = PIC2_Data;
        interrupt_number -= 8;
    }
    uint8_t interrupt_mask = read_port(pic);
    interrupt_mask &= ~(1 << interrupt_number);
    write_port(interrupt_mask, pic);
}
