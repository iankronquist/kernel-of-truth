#include <stdarg.h>

#include <truth/klog.h>
#include <truth/types.h>

#include <drivers/serial_port.h>

void init_logging(void) {
    status_t unused(stat) = device_char_init(klog_char_device, 1, NULL);
    device_char_puts(klog_char_device, "Logger initialized!\n");
}

void klog(char *message) {
    device_char_puts(klog_char_device, message);
}

static void klog_uint(uint64_t n, uint32_t length) {
    unsigned char buf[length];
    unsigned int c;
    // We explicitly abuse integer overflow!
    for (c = length-1; c < length; --c) {
        buf[c] = n % 16;
        n /= 16;
    }
    device_char_putc(klog_char_device, '0');
    device_char_putc(klog_char_device, 'x');
    ++c;
    for (; c < length; ++c) {
        if (buf[c] >= 10) {
            device_char_putc(klog_char_device, buf[c] + 'a' - 10);
        } else {
            device_char_putc(klog_char_device, buf[c] + '0');
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
                case '%':
                    device_char_putc(klog_char_device, '%');
                    break;
                case 'u':
                    klog_uint(va_arg(args, uint32_t), 8);
                    break;
                case 'd':
                case 'i':
                case 'p':
                case 'x':
                    if (i+1 == 'u') {
                        klog_uint(va_arg(args, uint32_t), 8);
                    }
                    break;
                case 'l':
                    if (i+1 == 'u') {
                        klog_uint(va_arg(args, uint32_t), 16);
                    }
                    break;
            }
            continue;
        }
        device_char_putc(klog_char_device, string[i]);
    }
    va_end(args);
}
