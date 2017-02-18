#include <truth/types.h>
#include <truth/string.h>

int memcmp(const void *a, const void *b, size_t size)
{
    const char *ac = (char *)a;
    const char *bc = (char *)b;

    for (size_t i = 0; i < size; i++) {
        if (ac[i] > bc[i]) {
            return -1;
        } else if (ac[i] < bc[i]) {
            return 1;
        }
    }
    return 0;
}

void *memcpy(void *destination, const void *source, size_t size)
{
    unsigned char *destinationc = (unsigned char *)destination;
    unsigned char *sourcec = (unsigned char *)source;
    for (size_t i = 0; i < size; i++) {
        destinationc[i] = sourcec[i];
    }
    return destination;
}

void *memset(void *buffer, int value, size_t size)
{
    unsigned char *bufferc = (unsigned char *) buffer;
    for (size_t i = 0; i < size; i++) {
        bufferc[i] = (unsigned char) value;
    }
    return buffer;
}

void *memmove(void *destination, const void *source, size_t size)
{
    unsigned char *dst = (unsigned char *) destination;
    const unsigned char *src = (const unsigned char *) source;
    if (dst < src) {
        for (size_t i = 0; i < size; i++) {
            dst[i] = src[i];
        }
    } else {
        for (size_t i = size; i != 0; i--) {
            dst[i-1] = src[i-1];
        }
    }
    return destination;
}

enum order strncmp(const char *s1, const char *s2, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        if (s1[i] != s2[i]) {
            if (s1[i] > s2[i]) {
                return Order_Greater;
            } else {
                return Order_Less;
            }
        }
    }
    return Order_Equal;
}

size_t strlen(const char *str) {
    size_t ret = 0;
    while (str[ret] != 0) {
        ret++;
    }
    return ret;
}
