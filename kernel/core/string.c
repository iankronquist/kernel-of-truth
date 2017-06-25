#include <truth/types.h>
#include <truth/string.h>
#include <truth/memory.h>

int memcmp(const void *a, const void *b, size_t size) {
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

int user_memcmp(const void *a, const void *b, size_t size) {
    memory_user_access_enable();
    int ret = memcmp(a, b, size);
    memory_user_access_disable();
    return ret;
}

void *memcpy(void *destination, const void *source, size_t size) {
    unsigned char *destinationc = (unsigned char *)destination;
    unsigned char *sourcec = (unsigned char *)source;
    for (size_t i = 0; i < size; i++) {
        destinationc[i] = sourcec[i];
    }
    return destination;
}

void *user_memcpy(void *destination, const void *source, size_t size) {
    memory_user_access_enable();
    void *ret = memcpy(destination, source, size);
    memory_user_access_disable();
    return ret;
}

void *memset(void *buffer, int value, size_t size) {
    unsigned char *bufferc = (unsigned char *) buffer;
    for (size_t i = 0; i < size; i++) {
        bufferc[i] = (unsigned char) value;
    }
    return buffer;
}

void *user_memset(void *buffer, int value, size_t size) {
    memory_user_access_enable();
    void *ret = memset(buffer, value, size);
    memory_user_access_disable();
    return ret;
}

void *memmove(void *destination, const void *source, size_t size) {
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

void *user_memmove(void *destination, const void *source, size_t size) {
    memory_user_access_enable();
    void *ret = memmove(destination, source, size);
    memory_user_access_disable();
    return ret;
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
        if (s1[i] == '\0' || s2[i] == '\0') {
            break;
        }
    }
    return Order_Equal;
}

enum order user_strncmp(const char *s1, const char *s2, size_t n) {
    memory_user_access_enable();
    enum order ret = strncmp(s1, s2, n);
    memory_user_access_disable();
    return ret;
}

size_t strlen(const char *str) {
    size_t ret = 0;
    while (str[ret] != 0) {
        ret++;
    }
    return ret;
}

size_t user_strlen(const char *str) {
    memory_user_access_enable();
    size_t ret = strlen(str);
    memory_user_access_disable();
    return ret;
}
