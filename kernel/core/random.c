#include <arch/x64/cpu.h>
#include <truth/entropy.h>
#include <truth/mt19937_64.h>
#include <truth/memory.h>
#include <truth/random.h>
#include <truth/types.h>
#include <truth/panic.h>

static uint64_t rdrand(void) {
    uint8_t carry = 0;
    uint64_t rand = 0;
    while (carry == 0) {
        __asm__ volatile ("rdrand %0; setc %1" : "+r"(rand), "=qm"(carry));
    }
    return rand;
}

static uint64_t (*random_number_function)(void) = NULL;

uint64_t random_number(void) {
    return random_number_function();
}

enum status random_init(void) {
    uint32_t eax, ebx, ecx, edx;
    eax = 1;
    cpuid(&eax, &ebx, &ecx, &edx);
    if (ecx & CPU_RDRAND) {
        random_number_function = rdrand;
    } else {
        union seed {
            uint8_t bytes[sizeof(uint64_t)];
            uint64_t qword;
        } seed;
        for (size_t i = 0; i < sizeof(uint64_t); ++i) {
            seed.bytes[i] = entropy_pool_consume();
        }
        mt19937_64_seed(seed.qword);
        random_number_function = mt19937_64_get_random_number;
    }
    return Ok;
}

void random_bytes(void *buf, size_t size) {
    uint64_t *word_buf = buf;
    uint8_t *byte_buf = buf;
    size_t remainder = size % sizeof(uint64_t);
    for (size_t i = 0; i < size / sizeof(uint64_t); ++i) {
        word_buf[i] = random_number();
    }
    if (remainder != 0) {
        uint64_t left_over = random_number();
        for (size_t i = 0; i < remainder; ++i) {
            byte_buf[size - remainder + i] = (left_over >> (i * sizeof(uint8_t))) & 0xff;
        }
    }
}


void *random_address(bool kernel_space, uint64_t alignment) {
    assert(is_power_of_two(alignment));
    uint64_t random = random_number();
    random = align_as(random, alignment);
    if (kernel_space) {
        random |= Memory_Kernel_Set_Mask;
    } else {
        random &= Memory_User_Clear_Mask;
    }
    return (void *)random;
}
