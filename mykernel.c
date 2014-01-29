#include <stddef.h>
#include <stdint.h>
 
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif
 
/* Hardware text mode color constants. */
enum vga_color
{
	COLOR_BLACK = 0,
	COLOR_LIGHT_GREY = 7,
};
/*
uint8_t make_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}
*/
 
uint16_t make_vgaentry(char c, uint8_t color)
{
	uint16_t c16 = c;
	uint16_t color16 = color;
	return c16 | color16 << 8;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 24;
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
 
void terminal_initialize()
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = 7;//make_color(COLOR_LIGHT_GREY, COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for ( size_t y = 0; y < VGA_HEIGHT; y++ )
	{
		for ( size_t x = 0; x < VGA_WIDTH; x++ )
		{
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = make_vgaentry(' ', terminal_color);
		}
	}
}
 
void terminal_setcolor(uint8_t color)
{
	terminal_color = color;
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = make_vgaentry(c, color);
}

void terminal_writestring_oflength(const char* data, uint16_t len);
void terminal_putchar(char c);
 
void terminal_putchar(char c)
{
	if(c == '\n')
	{
		terminal_column = 0;
		terminal_row++;
		terminal_row %= VGA_HEIGHT;
		return;
	}
	if(terminal_row >= VGA_HEIGHT)
	{
		terminal_row = 0;
		terminal_column = 0;
		
	}
	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if ( ++terminal_column == VGA_WIDTH )
	{
		terminal_column = 0;
		if ( ++terminal_row == VGA_HEIGHT )
		{
			terminal_row = 0;
		}
	}
}

void terminal_writestring_oflength(const char* data, uint16_t len)
{
	for ( size_t i = 0; i < len; i++ )
		terminal_putchar((char)data[i]);
}

 
void terminal_writestring(const char* data)
{
	for ( size_t i = 0; data[i] != '\0'; i++ )
		terminal_putchar(data[i]);
}

void terminal_writestring_oflength_2(uint8_t* data, uint8_t len)
{
	for ( size_t i = 0; i < len; i++ )
		terminal_putchar(data[i]);
}

void print_num(uint8_t num)
{
	
	terminal_putchar('0');
	terminal_putchar('b');
	for(int i = 0; i < 8; i++)
	{
		if(num & 1)
			terminal_putchar('1');
		else
			terminal_putchar('0');
		num = num >> 1;
	}
}
 
void kernel_main()
{
	terminal_initialize();
	char* str = 
"00aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n";
	for(int i = 0; i < 25; i++)
	{
		str[0]+=i;
		str[1]+=i;
		terminal_writestring(str);
	}
	//terminal_putentryat('0', 7,0,0);
	
	//terminal_writestring_oflength((char*)terminal_buffer[VGA_WIDTH], 
	//	(VGA_HEIGHT - 1) * VGA_WIDTH);
	uint8_t terminal_buffer_size = VGA_WIDTH * VGA_HEIGHT;

	/*for(int i = 1; i < 20 *VGA_HEIGHT; i++)
	{
		terminal_buffer[i] = terminal_buffer[i+VGA_WIDTH];
	}*/

/*	for(int i = 0; i < terminal_buffer_size; i++)
	{
		terminal_buffer[i] = terminal_buffer[(i + VGA_WIDTH) % terminal_buffer_size];
	}
	*/
	//terminal_row = 0;
	//terminal_column = 0;
	//char* str = terminal_buffer;
	//str[str+5] = '\0';
	//print_num(sizeof(char));
	//terminal_writestring_oflength((char*)terminal_buffer[], 25);
	//terminal_writestring_oflength(str, 
	//	20);
	//terminal_writestring_oflength("00000", 5);
}
