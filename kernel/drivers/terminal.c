#include <stddef.h>
#include <stdint.h>
#include <drivers/terminal.h>

uint8_t make_color(enum vga_color fg, enum vga_color bg)
{
    return fg | bg << 4;
}

uint16_t make_vgaentry(char c, uint8_t color)
{
    uint16_t c16 = c;
    uint16_t color16 = color;
    return c16 | color16 << 8;
}

void term_putentryat(char c, uint8_t color, size_t x, size_t y)
{
    const size_t index = y * VGA_WIDTH + x;
    term_buffer[index] = make_vgaentry(c, color);
}

void term_initialize()
{
    term_row = 0;
    term_column = 0;
    term_color = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    term_buffer = (uint16_t*)0xB8000;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            term_buffer[index] = make_vgaentry(' ', term_color);
        }
    }
}

int term_putchar(char c)
{
    if (c == '\n') {
        term_column = 0;

        if(++term_row == VGA_HEIGHT) {
            term_copy_up_lines();
        }

        return c;
    }

    if(term_column >= VGA_WIDTH) {
        term_color = 0;
        term_row++;
    }

    term_putentryat(c, term_color, term_column, term_row);
    if (++term_column == VGA_WIDTH) {
        term_column = 0;
        if (++term_row == VGA_HEIGHT) {
            term_copy_up_lines();
        }
    }
    return c;
}

int term_writestring(char* data)
{
    size_t i;
    for (i = 0; data[i] != 0; i++)
        term_putchar(data[i]);
    return data[i];
}

void term_copy_up_lines()
{
    for(size_t y = 0; y < VGA_HEIGHT; y++)
    {
        for(size_t x = 0; x < VGA_WIDTH; x++)
        {
            size_t index_to = y*VGA_WIDTH+x;
            size_t index_from = (y+1)*VGA_WIDTH+x;

            if(x == VGA_HEIGHT)
            {
                term_buffer[index_to] = make_vgaentry(' ', term_color);
            }
            else
            {
                term_buffer[index_to] = term_buffer[index_from];
            }
        } 
    }

    term_row = VGA_HEIGHT-1;
    term_column = 0;
}
