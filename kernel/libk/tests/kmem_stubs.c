#include <stdlib.h>
#include <stddef.h>

void *kmalloc(size_t bytes) {
    return malloc(bytes);
}

void *krealloc(void *ptr, size_t bytes) {
    return realloc(ptr, bytes);
}

void kfree(void *ptr) {
    free(ptr);
}
