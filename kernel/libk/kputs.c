#include <libk/kputs.h>

void kputs(char* string)
{
    terminal_writestring(string);
}

void kprint_int(char* string, int i)
{
    terminal_writestring(string);
    do {
        terminal_putchar((i % 10) + '0');
        i /= 10;
    } while (i != 0);
}
