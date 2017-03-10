#include <arch/x64/interrupts.h>
#include <arch/x64/pic.h>
#include <arch/x64/port.h>
#include <truth/interrupts.h>
#include <truth/log.h>

#define Timer_IRQ_Number 0x20

#define Timer_Magic_Number 1193180

#define Timer_Chan_0 0x40
#define Timer_Chan_1 0x41
#define Timer_Chan_2 0x42
#define Timer_Command_Reg 0x43
#define RW_Oneshot_Square 0x36

static void timer_interrupt_handler(struct interrupt_cpu_state *unused(r)) {
    log(Log_Info, "tick");
}

void timer_init(void) {
    interrupt_register_handler(Timer_IRQ_Number, timer_interrupt_handler);
}

void timer_set_phase(uint8_t hertz) {
    uint8_t divisor = Timer_Magic_Number / hertz;
    write_port(Timer_Command_Reg, RW_Oneshot_Square);
    write_port(Timer_Chan_0, divisor & 0xff);
    write_port(Timer_Chan_0, divisor >> 8);
    pic_enable(Timer_IRQ_Number);
}

void timer_fini(void) {
    interrupt_unregister_handler(Timer_IRQ_Number, timer_interrupt_handler);
}
