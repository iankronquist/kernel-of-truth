#include <truth/random.h>
#include <truth/types.h>

static uint64_t rdrand(void) {
    uint8_t carry = 0;
    uint64_t rand = 0;
    while (carry == 0) {
        __asm__ volatile ("rdrand %0; setc %1" : "+r"(rand), "=qm"(carry));
    }
    return rand;
}

void random(void *buf, size_t size) {
    uint64_t *word_buf = buf;
    uint8_t *byte_buf = buf;
    size_t remainder = size % sizeof(uint64_t);
    for (size_t i = 0; i < size / sizeof(uint64_t); ++i) {
        word_buf[i] = rdrand();
    }
    if (remainder != 0) {
        uint64_t left_over = rdrand();
        for (size_t i = 0; i < remainder; ++i) {
            byte_buf[i] = (left_over >> (i * sizeof(uint8_t))) & 0xff;
        }
    }
}
