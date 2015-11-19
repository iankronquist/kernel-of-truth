#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <stddef.h>

#include <arch/x86/memlayout.h>

/* Hardware text mode color constants. */
enum vga_color {
    COLOR_BLACK = 0,
    COLOR_BLUE = 1,
    COLOR_GREEN = 2,
    COLOR_CYAN = 3,
    COLOR_RED = 4,
    COLOR_MAGENTA = 5,
    COLOR_BROWN = 6,
    COLOR_LIGHT_GREY = 7,
    COLOR_DARK_GREY = 8,
    COLOR_LIGHT_BLUE = 9,
    COLOR_LIGHT_GREEN = 10,
    COLOR_LIGHT_CYAN = 11,
    COLOR_LIGHT_RED = 12,
    COLOR_LIGHT_MAGENTA = 13,
    COLOR_LIGHT_BROWN = 14,
    COLOR_WHITE = 15,
};

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t term_row;
size_t term_column;
uint8_t term_color;
uint16_t* term_buffer;

void term_copy_up_lines();
int term_writestring(char* data);
int term_putchar(char c);
void terminal_deletechar();
void term_initialize();
void term_putentryat(char c, uint8_t color, size_t x, size_t y);
void terminal_deleteentrat(size_t x, size_t y);
uint16_t make_vgaentry(char c, uint8_t color);
uint8_t make_color(enum vga_color fg, enum vga_color bg);

#endif
