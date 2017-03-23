#include <truth/random.h>

void randombytes_buf(void * const buf, const size_t size) {
    random_bytes(buf, size);
}
