#include <arch/x64/port.h>

#include <truth/log.h>

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


void pic_end_of_interrupt(int interrupt_number) {
    if (interrupt_number > PIC2_First_Interrupt) {
        write_port(PIC2_Control, PIC_End_Of_Interrupt);
    }
    write_port(PIC1_Control, PIC_End_Of_Interrupt);
}


void pic_init(void) {
    write_port(PIC1_Control, ICW1_Init | ICW1_ICW4_Disable);
    write_port(PIC2_Control, ICW1_Init | ICW1_ICW4_Disable);

    write_port(PIC1_Data, PIC1_Offset);
    write_port(PIC2_Data, PIC2_Offset);

    write_port(PIC1_Data, PIC1_Next_PIC);
    write_port(PIC2_Data, PIC2_Identity);

    write_port(PIC1_Data, ICW4_8086_Mode);
    write_port(PIC2_Data, ICW4_8086_Mode);
}


void pic_enable_all(void) {
    write_port(PIC1_Data, 0xff);
    write_port(PIC2_Data, 0xff);
}
