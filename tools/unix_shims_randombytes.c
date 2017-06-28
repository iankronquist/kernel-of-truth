#include <assert.h>
#include <stdio.h>

void randombytes_buf(void * const buf, const size_t size) {
    FILE *urandom = fopen("/dev/urandom", "r");
    assert(urandom != NULL);
    int count = fread(buf, 1, size, urandom);
    assert(count == (int)size);
    fclose(urandom);
}
