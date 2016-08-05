#include <libk/klog.h>

void initialize_klog() {
    initialize_serial_port(COM1);
    write_serial_string(COM1, "Logger initialized!\n");
}
void klog(char *message) {
    write_serial_string(COM1, message);
}

static void klog_uint(uint64_t n, uint32_t length) {
    unsigned char buf[length];
    unsigned int c;
    // We explicitly abuse integer overflow!
    for (c = length-1; c < length; --c) {
        buf[c] = n % 16;
        n /= 16;
    }
    write_serial(COM1, '0');
    write_serial(COM1, 'x');
    ++c;
    for (; c < length; ++c) {
        if (buf[c] >= 10) {
            write_serial(COM1, buf[c] + 'a' - 10);
        } else {
            write_serial(COM1, buf[c] + '0');
        }
    }
}

// A subset of printf
void klogf(char* string, ...) {
    va_list args;
    va_start(args, string);
    for (size_t i = 0; string[i] != '\0'; ++i) {
        if (string[i] == '%') {
            ++i;
            switch(string[i]) {
                case 's':
                    write_serial_string(COM1, va_arg(args, char*));
                    break;
                case '%':
                    write_serial(COM1, '%');
                    break;
                case 'u':
                    klog_uint(va_arg(args, uint32_t), 8);
                    break;
                case 'd':
                case 'i':
                    if (i+1 == 'u') {
                        klog_uint(va_arg(args, uint32_t), 8);
                    }
                case 'p':
                case 'x':
                    klog_uint(va_arg(args, uint32_t), 8);
                    break;
                case 'l':
                    if (i+1 == 'u') {
                        klog_uint(va_arg(args, uint32_t), 16);
                    }
                    break;
            }
            continue;
        }
        write_serial(COM1, string[i]);
    }
    va_end(args);
}
