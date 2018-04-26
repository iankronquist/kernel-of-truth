#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


_Static_assert(sizeof(long) >= sizeof(uint32_t),
        "long must be able to hold a uint32_t");


int usage(char *name) {
    fprintf(stderr, "Usage: %s leaf_number\n"
            "\tPrint the output of CPUID for the provided\n", name);
    return EXIT_FAILURE;
}


static inline void cpuid(uint32_t *eax, uint32_t *ebx, uint32_t *ecx,
        uint32_t *edx) {
    __asm__ volatile ("cpuid" :
            "=a"(*eax),
            "=b"(*ebx),
            "=c"(*ecx),
            "=d"(*edx)
            : "0" (*eax)
            :);
}


int main(int argc, char **argv) {
    long leaf;
    uint32_t eax, ebx, ecx, edx;
    if(argc != 2) {
        return usage(argv[0]);
    }

    leaf = strtol(argv[1], NULL, 0);
    printf("%ld\n", leaf);
    if(errno == EINVAL) {
        fprintf(stderr, "Invalid leaf: %s\n", argv[1]);
        return EXIT_FAILURE;
    } else if(errno == ERANGE || leaf > INT32_MAX || leaf < INT32_MIN) {
        fprintf(stderr, "Leaf can't fit in 32 bit integer: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    eax = (uint32_t)leaf;
    cpuid(&eax, &ebx, &ecx, &edx);

    printf("EAX: %08x\nEBX: %08x\nECX: %08x\nEDX: %08x\n", eax, ebx, ecx, edx);

    return EXIT_SUCCESS;
}
