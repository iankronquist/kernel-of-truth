#include <truth/types.h>
#include <truth/file.h>
#include <truth/log.h>
#include <truth/panic.h>

// Here there be dragons. Go away.

#define buf_size 256

static enum status checked print_number(struct file *file, char *buf,
                                        size_t *top, bool is_signed,
                                        uint8_t base, uint8_t size,
                                        int64_t snumber) {
    char digits_safe[] = "BUG0123456789abcdefBUG";
    char *digits = &digits_safe[3];

    if (is_signed && snumber < 0) {
        buf[*top] = '-';
        (*top)++;
        snumber = ~snumber + 1;
    }
    uint64_t number = (uint64_t)snumber;
    // Clear the buffer because we're going to need some space.
    bubble(file->write((byte *)buf, *top), "Clearing buffer in print_number");

    switch (base) {
        case 2:
            *top = size * 8 / 1;
            break;
        case 8:
            *top = size * 8 / 2;
            break;
        case 10:
            // log_2(10) = 3.3219, which rounds up to four.
            *top = size * 8 / 4;
            break;
        case 16:
            *top = size * 8 / 4;
            break;
        default:
            return Error_Invalid;
    }

    // assert(*top <= buf_size);

    for (size_t i = *top; i != 0; --i) {
        buf[i-1] = digits[number%base];
        number /= base;
    }
    return Ok;
}

static enum status checked print_string(struct file *file, char *buf,
                                        size_t *top, const char *string) {
    for (size_t i = 0; string[i] != '\0'; ++i) {
        buf[*top] = string[i];
        (*top)++;
        if (*top == buf_size) {
            bubble(file->write((byte *)buf, buf_size),
                   "Clearing buffer in print_string");
            *top = 0;
        }
    }
    return Ok;
}

/* A varargs version of formatted print to file with a wide variety of format
 * specifiers.
 * Input is buffered on the stack an periodically flushed to the file.
 * We support the following specifiers:
 * - %%: A single '%' character.
 * - %i and %d: A decimal integer, by default signed.
 * - %u: A decimal integer, this time unsigned.
 * - %x: A hexadecimal unsigned integer in lower case.
 * - %X: Same as above. Capital letters are ugly.
 * - %zu: A size_t, unsigned or course. Printed in hexadecimal because that's
 *        useful to me.
 * - %c: A single character.
 * - %s: A C style string.
 * - %p: A void* pointer.
 * - %b: A unsigned binary integer.
 *
 * Additionally we support the following modifiers:
 * - l: A 32 bit integer.
 * - ll: A 64 bit integer.
 * - h: A 16 bit integer
 * - hh: An 8 bit integer
 *
 * We do not support wide characters. We do not support %n. We do not support
 * any floating point specifiers.
 * We always fill leading zeros.
 *
 */
enum status vfprintf(struct file *file, const char *restrict format,
                     va_list args) {
    size_t top = 0;
    char buf[buf_size];
    if (file->write == NULL) {
        return Error_Permissions;
    }
    for (size_t i = 0; format[i] != '\0'; ++i) {
        if (format[i] == '%' && format[i+1] != '\0') {
            int64_t number = 0;
            bool is_signed = true;
            uint8_t base;
            uint8_t size = 0;
            switch (format[i+1]) {
                case 'l':
                    i++;
                    if (format[i+1] == 'l') {
                        i++;
                        size = sizeof(long long);
                    } else {
                        size = sizeof(long);
                    }
                    break;
                case 'h':
                    i++;
                    if (format[i+1] == 'h') {
                        i++;
                        size = sizeof(char);
                    } else {
                        size = sizeof(short);
                    }
                    break;
                case 'z':
                    i++;
                    if (format[i+1] != 'u') {
                        bubble(print_string(file, buf, &top, "%z"),
                               "Clearing buffer after print_string");
                    } else {
                        i++;
                        is_signed = false;
                        size = sizeof(size_t);
                        number = va_arg(args, size_t);
                        base = 10;
                        bubble(print_number(file, buf, &top, is_signed, base,
                                            size, number),
                               "Clearing buffer after print_number");
                    }
                    goto next_iteration;
            }
            switch (format[i+1]) {
                case 'i':
                // fall through.
                case 'd':
                    i++;
                    base = 10;
                    break;
                case 'u':
                    i++;
                    base = 10;
                    is_signed = false;
                    break;
                case 'p':
                    i++;
                    if (size == 0) {
                        size = sizeof(void *);
                    }
                    is_signed = false;
                    base = 16;
                    break;
                case 'X':
                // We don't support capitalized hexadecimal because caps
                // are ugly.
                // fall through.
                case 'x':
                    i++;
                    is_signed = false;
                    base = 16;
                    break;
                case 'o':
                    i++;
                    base = 8;
                    break;
                case 'b':
                    i++;
                    base = 2;
                    is_signed = false;
                    break;
                case 'c':
                    i++;
                    int character;
                    character = va_arg(args, int);
                    buf[top] = (char)character;
                    top++;
                    goto next_iteration;
                case 's':
                    i++;
                    char *str;
                    str = va_arg(args, char *);
                    if (str == NULL) {
                        str = "(NULL)";
                    }
                    bubble(print_string(file, buf, &top, str),
                           "clearing buffer after print_string");
                    goto next_iteration;
                case '%':
                    i++;
                    buf[top] = '%';
                    top++;
                    goto next_iteration;
                // fall through.
                default:
                    // Invalid specifier, just print a '%'.
                    buf[top] = format[i];
                    top++;
                    goto next_iteration;
            }

            switch (size) {
                // Default size.
                case 0:
                    size = sizeof(int);
                    number = va_arg(args, int);
                    break;
                // In var args chars and shorts are automatically promoted to
                // ints.
                case sizeof(char):
                case sizeof(short):
                case sizeof(int):
                    number = va_arg(args, int);
                    break;
                case sizeof(long):
                    number = va_arg(args, long);
                    break;
            }
            bubble(print_number(file, buf, &top, is_signed, base, size,
                                number), "Clearing buffer after print_number");
        } else {
            buf[top] = format[i];
            top++;
        }
next_iteration:
        // If we have filled the buffer, flush it.
        if (top == buf_size) {
            bubble(file->write((byte *)buf, buf_size),
                   "Clearing buffer in vfprintf");
        }
    }
    // Flush anything remaining in the buffer.
    return file->write((byte *)buf, top);
}

enum status checked fprintf(struct file *file,
                            const char *restrict format, ...) {
    va_list args;
    va_start(args, format);
    enum status status = vfprintf(file, format, args);
    va_end(args);
    return status;
}
