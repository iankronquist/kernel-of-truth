ARCH=i686-elf
TEST_CC=clang
CC=compiler/$(ARCH)/bin/$(ARCH)-gcc
AS=compiler/$(ARCH)/bin/$(ARCH)-as
CFLAGS= -std=c99 -ffreestanding -O0 -Wall -Wextra -g

all: bootloader-x86 kernel link-x86 

bootloader-x86: build
	${AS} kernel/arch/x86/boot.s -o build/boot.o

kernel: terminal gdt-x86 idt-x86 tmem build
	${CC} -c kernel/kernel.c -o build/kernel.o  ${CFLAGS}
	${CC} -c kernel/kassert.c -o build/kassert.o  ${CFLAGS}
	${CC} -c kernel/kputs.c -o build/kputs.o  ${CFLAGS}
	${CC} -c kernel/kabort.c -o build/kabort.o  ${CFLAGS}

link-x86: build
	${CC} -T kernel/arch/x86/linker.ld -o build/truthos.bin -ffreestanding -O0 -nostdlib build/boot.o build/kernel.o build/terminal.o build/gdts.o build/idts.o build/gdtc.o build/idtc.o build/tmem.o build/kabort.o build/kassert.o build/kputs.o -lgcc

terminal: build
	${CC} -c kernel/terminal.c -o build/terminal.o ${CFLAGS}

gdt-x86: build
	${CC} -c kernel/arch/x86/gdt.c -o build/gdtc.o ${CFLAGS}
	${AS} kernel/arch/x86/gdt.s -o build/gdts.o

idt-x86: build
	${CC} -c kernel/arch/x86/idt.c -o build/idtc.o ${CFLAGS}
	${AS} kernel/arch/x86/idt.s -o build/idts.o

tmem: build
	${CC} -c tlibc/tmem/mem.c -o build/tmem.o ${CFLAGS}

start:
	qemu-system-i386 -kernel build/truthos.bin

start-log:
	qemu-system-i386 -kernel build/truthos.bin -d in_asm,cpu_reset,exec,int,op,guest_errors,pcall -no-reboot 2> qemu.log

start-debug:
	qemu-system-i386 -S -s -kernel build/truthos.bin

build:
	mkdir build

clean:
	rm -rf build
