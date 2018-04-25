#pragma once
#include <truth/types.h>

enum status vga_init(void);
void vga_putc(const char str);
void vga_log_putc(const char c);
