#include <loader/string.h>

// FIXME: This is inefficient.
void *memset(void *b, int c, size_t len) {
    uint8_t *buf = b;
    for (size_t i = 0; i < len; ++i) {
        buf[i] = c;
    }
    return b;
}

size_t strlen(const char *str) {
    const char *c;
    for (c = str; *c != '\0'; ++c) { }
    return c - str;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    for (size_t i = 0; i < n && *s1 != '\0' && *s2 != '\0'; ++s1, ++s2, ++i) {
        int diff = *s1 - *s2;
        if (diff != 0) {
            return diff;
        }
    }
    return *s1 - *s2;
}
