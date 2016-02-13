#include <drivers/timer.h>

void timer_install() {
    uint8_t current_mask = read_port(0x21);
    write_port(0x21 , current_mask & TIMER_INTERRUPT_MASK);
}

void set_timer_phase(uint8_t hertz) {
    uint8_t divisor = TIMER_MAGIC_NUMBER / hertz;
    write_port(TIMER_COMMAND_REG, RW_ONESHOT_SQUARE);
    // Write low bits
    write_port(TIMER_CHAN_0, divisor & 0xff);
    // Write high bits
    write_port(TIMER_CHAN_0, divisor >> 8);
}

void timer_irq_handler() {
    sys_kputs("tick!");
    // End interrupt
    write_port(0x20, 0x20);
}
