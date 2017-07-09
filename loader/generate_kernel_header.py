import sys

prefix = '''#pragma once
#include <stdint.h>
uint8_t Kernel_ELF[]__attribute__((aligned(0x1000)))={'''

suffix = '''};
'''

if __name__ == '__main__':
    if len(sys.argv) != 3:
        sys.stderr.write('Usage: {} elf_file header_file\n'.format(sys.argv[0]))
        exit(-1)

    with open(sys.argv[1], 'w') as header:
        header.write(prefix)
        with open(sys.argv[2], 'r') as elf:
            for byte in elf.read():
                header.write('{},'.format(ord(byte)))
        header.write(suffix)
