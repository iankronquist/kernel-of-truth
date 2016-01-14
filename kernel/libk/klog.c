#include <libk/klog.h>

void initialize_klog() {
    initialize_serial_port(COM1);
    write_serial_string(COM1, "Logger initialized!\n");
}

void sys_klog(char *message) {
    write_serial_string(COM1, message);
}

static void klog_u32(uint32_t n) {
    unsigned char buf[8] = {0};
    int c;
    for (c = 7; c >= 0; --c) {
        buf[c] = n % 16;
        n /= 16;
    }
    write_serial(COM1, '0');
    write_serial(COM1, 'x');
    ++c;
    for (; c < 8; ++c) {
        if (buf[c] >= 10) {
            write_serial(COM1, buf[c] + 'a' - 10);
        } else {
            write_serial(COM1, buf[c] + '0');
        }
    }
}

// A subset of printf
void sys_klogf(char* string, ...) {
    va_list args;
    va_start(args, string);
    for (size_t i = 0; string[i] != '\0'; ++i) {
        if (string[i] == '%') {
            ++i;
            switch(string[i]) {
                case '%':
                    write_serial(COM1, '%');
                    break;
                case 'p':
                    klog_u32(va_arg(args, uint32_t));
                    break;
            }
            continue;
        }
        write_serial(COM1, string[i]);
    }
    va_end(args);
}
