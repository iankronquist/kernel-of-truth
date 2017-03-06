#pragma once

void pic_end_of_interrupt(uint8_t interrupt_number);
void pic_init(void);
void pic_enable_exceptions(void);
void pic_enable(uint8_t interrupt_number);
void pic_disable(uint8_t interrupt_number);
