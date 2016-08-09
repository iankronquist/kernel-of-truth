#include <drivers/terminal.h>

#include <truth/device.h>
#include <truth/types.h>

#include <truth/private/memlayout.h>

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
static size_t terminal_row;
// The current cursor column.
static size_t terminal_column;
// The current cursor color.
static uint8_t terminal_color;
// A pointer to the terminal buffer.
static uint16_t* terminal_buffer;

// Scroll all of the text on the VGA console up one line.
static void terminal_scroll(void);
// Write a single character to the VGA console.
static int terminal_putchar(char c);
// Delete a single character on the terminal -- does not handle newlines!
static void terminal_deletechar(void);

// Initialize the VGA terminal drivers.
void terminal_initialize(void);
// Put a character with the provided color at the given location.
static void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
// Erase a character at the given location.
static void terminal_deleteentryat(size_t x, size_t y);
// Make a VGA entry for writing to the screen.
// Given a character and a color, @return a VGA entry which can be displayed by
// the hardware.
static uint16_t make_vgaentry(char c, uint8_t color);
// Make a VGA color.
static uint8_t make_color(enum vga_color fg, enum vga_color bg);


static uint8_t make_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static uint16_t make_vgaentry(char c, uint8_t color) {
    uint16_t c16 = c;
    uint16_t color16 = color;
    return c16 | color16 << 8;
}

static void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = make_vgaentry(c, color);
}

static void terminal_deleteentryat(size_t x, size_t y) {
    const size_t index = y*VGA_WIDTH+x;
    terminal_buffer[index] = make_vgaentry(' ', terminal_color);
}

static void terminal_clear(void) {
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

static int terminal_putchar(char c) {
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

static void terminal_deletechar() {
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

static void terminal_scroll(void) {
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

status_t checked term_dev_init(const struct device *unused(dev),
        int unused(device_number), void *unused(args)) {
    terminal_clear();
    return Ok;
}

void term_dev_fini(const struct device *unused(dev)) {
    terminal_clear();
}

ssize_t term_dev_write(const struct device *unused(dev), char *buf, size_t size) {
    ssize_t i;
    for (i = 0; i < (ssize_t)size; i++)
        terminal_putchar(buf[i]);
    return i;
}

void terminal_initialize(void) {
    status_t unused(stat) = register_device(terminal_char_device);
    terminal_clear();
}

int term_dev_putc(const struct device *unused(dev), char c) {
    return terminal_putchar(c);
}

struct device vga_char_device = {
    .number = 0,
    .name = "tty0",
    .type = char_device,
    .character = {
        .fini = term_dev_fini,
        .init = term_dev_init,
        .write = term_dev_write,
        .write_char = term_dev_putc,
        .read = NULL,
        .read_char = NULL,
    },
};

struct device *terminal_char_device = &vga_char_device;
