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

void random_bytes(void *buf, size_t size) {
    uint64_t *word_buf = buf;
    uint8_t *byte_buf = buf;
    size_t remainder = size % sizeof(uint64_t);
    for (size_t i = 0; i < size / sizeof(uint64_t); ++i) {
        word_buf[i] = rdrand();
    }
    if (remainder != 0) {
        uint64_t left_over = rdrand();
        for (size_t i = 0; i < remainder; ++i) {
            byte_buf[size - remainder + i] = (left_over >> (i * sizeof(uint8_t))) & 0xff;
        }
    }
}


void *random_address(bool kernel_space, uint64_t alignment) {
    assert(is_power_of_two(alignment));
    uint64_t random = ((uint64_t)randombytes_random() << 32) | randombytes_random();
    random = align_as(random, alignment);
    if (kernel_space) {
        random |= Memory_Kernel_Set_Mask;
    } else {
        random &= Memory_User_Clear_Mask;
    }
    return (void *)random;
}
