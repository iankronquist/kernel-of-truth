#include <libk/kputs.h>

void sys_kputs(char* string) {
    terminal_writestring(string);
    terminal_putchar('\n');
}

static void kprint_ptr(void *p) {
    unsigned char buf[8] = {0};
    int c;
    uintptr_t n = (uintptr_t)p;
    for (c = 7; c >= 0; --c) {
        buf[c] = n % 16;
        n /= 16;
    }
    terminal_putchar('0');
    terminal_putchar('x');
    ++c;
    for (; c < 8; ++c) {
        if (buf[c] >= 10) {
            terminal_putchar(buf[c] + 'a' - 10);
        } else {
            terminal_putchar(buf[c] + '0');
        }
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
        terminal_putchar(buf[c]+'0');
    }
}

static void kprint_int(int i) {
    if (i < 0) {
        terminal_putchar('-');
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
        terminal_putchar(buf[c]+'0');
    }
}


// A subset of printf
void sys_kprintf(char* string, ...) {
    va_list args;
    va_start(args, string);
    for (size_t i = 0; string[i] != '\0'; ++i) {
        if (string[i] == '%') {
            ++i;
            switch(string[i]) {
                case '%':
                    terminal_putchar('%');
                    break;
                case 'p':
                    kprint_ptr(va_arg(args, void*));
                    break;
                case 'd':
                case 'i':
                    kprint_int(va_arg(args, int));
                    break;
                case 's':
                    terminal_writestring(va_arg(args, char*));
                    break;
                case 'u':
                    kprint_uint(va_arg(args, unsigned int));
                    break;
                case 'c':
                    terminal_putchar(va_arg(args, int));
                    break;
            }
            continue;
        }
        terminal_putchar(string[i]);
    }
    va_end(args);
}
