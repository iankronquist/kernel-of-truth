#include <truth/random.h>
#include <truth/string.h>

void randombytes_buf(void * const buf, const size_t size) {
    random(buf, size);
}

void sodium_memzero(void *const pnt, const size_t len) {
    memset(pnt, 0, len);
}

int sodium_memcmp(const void *const b1_, const void *const b2_, size_t len) {
    return memcmp(b1_, b2_, len);
}
