TEST_CC=clang
CC=../bin/bin/i586-elf-gcc
AS=../bin/bin/i586-elf-as
CFLAGS= -std=c99 -ffreestanding -O2 -Wall -Wextra

all: bootloader-i586 kernel link-i586 

bootloader-i586: build
	${AS} kernel/arch/i586/boot.s -o build/boot.o

kernel: terminal gdt-i586 idt-i586 isr-i586 tmem build
	${CC} -c kernel/kernel.c -o build/kernel.o  ${CFLAGS}

link-i586: build
	${CC} -T kernel/arch/i586/linker.ld -o build/truthos.bin -ffreestanding -O2 -nostdlib build/boot.o build/kernel.o build/terminal.o build/gdt.o build/idt.o build/isr.o build/gdtc.o build/isrc.o build/idtc.o build/tmem.o -lgcc

terminal: build
	${CC} -c kernel/terminal.c -o build/terminal.o ${CFLAGS}

gdt-i586: build
	${CC} -c kernel/gdt.c -o build/gdtc.o ${CFLAGS}
	${AS} kernel/arch/i586/gdt.s -o build/gdt.o

idt-i586: build
	${CC} -c kernel/idt.c -o build/idtc.o ${CFLAGS}
	${AS} kernel/arch/i586/idt.s -o build/idt.o

isr-i586: build
	${CC} -c kernel/isr.c -o build/isrc.o ${CFLAGS}
	${AS} kernel/arch/i586/isr.s -o build/isr.o

tmem: build
	${CC} -c tlibc/tmem/mem.c -o build/tmem.o ${CFLAGS}

start:
	qemu-system-i386 -kernel build/truthos.bin

build:
	mkdir build

clean:
	rm -rf build

