#include <libk/kputs.h>

void kputs(char* string) {
    term_writestring(string);
    term_putchar('\n');
}

static void kprint_ptr(void *p) {
    unsigned char buf[10] = {0};
    unsigned short number;
    unsigned int c;
    uintptr_t n = (uintptr_t)p;
    for (c = 9; c > 0; --c) {
        number = (n % 16) + '0';
        if (number >= 10) {
            number += 'a' - 10;
        } else {
            number += '0';
        }
        buf[c] = number;
        n /= 16;
    }
    term_putchar('0');
    term_putchar('x');
    for (; c < 10; ++c) {
        term_putchar(buf[c]+'0');
    }
}

static void kprint_uint(unsigned int i) {
    unsigned char buf[10] = {0};
    unsigned int c;
    for (c = 9; c > 0; --c) {
        buf[c] = (i % 10);
        i /= 10;
        if (i == 0) {
            break;
        }
    }
    for (; c < 10; ++c) {
        term_putchar(buf[c]+'0');
    }
}

static void kprint_int(int i) {
    if (i < 0) {
        term_putchar('-');
    }
    unsigned char buf[10] = {0};
    unsigned int c;
    for (c = 9; c > 0; --c) {
        buf[c] = (i % 10);
        i /= 10;
        if (i == 0) {
            break;
        }
    }
    for (; c < 10; ++c) {
        term_putchar(buf[c]+'0');
    }
}


// A subset of printf
void kprintf(char* string, ...) {
    va_list args;
    va_start(args, string);
    for (size_t i = 0; string[i] != '\0'; ++i) {
        if (string[i] == '%') {
            ++i;
            switch(string[i]) {
                case '%':
                    term_putchar('%');
                    break;
                case 'p':
                    kprint_ptr(va_arg(args, void*));
                    break;
                case 'd':
                case 'i':
                    kprint_int(va_arg(args, int));
                    break;
                case 's':
                    term_writestring(va_arg(args, char*));
                    break;
                case 'u':
                    kprint_uint(va_arg(args, unsigned int));
                    break;

            }
            continue;
        }
        term_putchar(string[i]);
    }
    va_end(args);
}
