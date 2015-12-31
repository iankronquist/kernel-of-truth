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

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = make_vgaentry(c, color);
}

void terminal_deleteentryat(size_t x, size_t y) {
    const size_t index = y*VGA_WIDTH+x;
    terminal_buffer[index] = make_vgaentry(' ', terminal_color);
}

void terminal_initialize()
{
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    terminal_buffer = (uint16_t*)VIDEO_MEMORY_BEGIN;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = make_vgaentry(' ', terminal_color);
        }
    }
}

int terminal_putchar(char c)
{
    if (c == '\n') {
        terminal_column = 0;

        if(++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }

        return c;
    }

    if(c == '\b') {
        terminal_deletechar();
        return c;
    }

    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
    }
    return c;
}

void terminal_deletechar() {
	size_t size_t_max = (size_t)-1;
	
	if(--terminal_column == size_t_max) {
		if(terminal_row == 0) {
			terminal_column = 0;
		} else {
			terminal_column = VGA_WIDTH-1;
		}

		if(--terminal_row == size_t_max) {
			terminal_row = 0;
		}
	}

    terminal_deleteentryat(terminal_column, terminal_row);
}

int terminal_writestring(char* data)
{
    size_t i;
    for (i = 0; data[i] != 0; i++)
        terminal_putchar(data[i]);
    return data[i];
}

void terminal_scroll()
{
    // Copy up lines
    for(size_t y = 0; y < VGA_HEIGHT-1; y++)
    {
        for(size_t x = 0; x < VGA_WIDTH; x++)
        {
            size_t index_to = y*VGA_WIDTH+x;
            size_t index_from = (y+1)*VGA_WIDTH+x;

            terminal_buffer[index_to] = terminal_buffer[index_from];
        }
    }

    // Clear last line
    int16_t blank = make_vgaentry(' ', terminal_color);
    for(size_t x = 0; x < VGA_WIDTH; x++)
    {
        terminal_buffer[x + VGA_WIDTH * (VGA_HEIGHT - 1)] = blank;
    }

    // Reset cursor to the beginning of the last line
    terminal_row = VGA_HEIGHT-1;
    terminal_column = 0;
}

void terminal_writebuffer() {
	return;
}
