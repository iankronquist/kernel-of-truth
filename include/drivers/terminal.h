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

// The width of the VGA console.
static const size_t VGA_WIDTH = 80;
// The height of the VGA console.
static const size_t VGA_HEIGHT = 25;

// The current cursor row.
size_t terminal_row;
// The current cursor column.
size_t terminal_column;
// The current cursor color.
uint8_t terminal_color;
// A pointer to the terminal buffer.
uint16_t* terminal_buffer;

// Scroll all of the text on the VGA console up one line.
void terminal_scroll();
// Write a string to the VGA console.
// The string will be written to
int terminal_writestring(char* data);
// Write a single character to the VGA console.
int terminal_putchar(char c);
// Delete a single character on the terminal -- does not handle newlines!
void terminal_deletechar(void);

// Initialize the VGA terminal drivers.
void terminal_initialize(void);
// Put a character with the provided color at the given location.
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
// Erase a character at the given location.
void terminal_deleteentrat(size_t x, size_t y);
// Make a VGA entry for writing to the screen.
// Given a character and a color, @return a VGA entry which can be displayed by
// the hardware.
uint16_t make_vgaentry(char c, uint8_t color);
// Make a VGA color.
uint8_t make_color(enum vga_color fg, enum vga_color bg);

#endif
