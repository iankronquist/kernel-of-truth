#include <libk/kputs.h>

void kputs(char* string)
{
    term_writestring(string);
}

void kprint_int(char* string, int i)
{
    term_writestring(string);
    do {
        term_putchar((i % 10) + '0');
        i /= 10;
    } while (i != 0);
}
