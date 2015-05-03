ARCH=i686-elf
TEST_CC=clang
CC=compiler/$(ARCH)/bin/$(ARCH)-gcc
AS=compiler/$(ARCH)/bin/$(ARCH)-as
CFLAGS= -std=c99 -ffreestanding -O2 -Wall -Wextra

all: bootloader-i686 kernel link-i686 

bootloader-i686: build
	${AS} kernel/arch/i586/boot.s -o build/boot.o

kernel: terminal gdt-i686 idt-i686 isr-i686 tmem build
	${CC} -c kernel/kernel.c -o build/kernel.o  ${CFLAGS}
	${CC} -c kernel/kassert.c -o build/kassert.o  ${CFLAGS}
	${CC} -c kernel/kputs.c -o build/kputs.o  ${CFLAGS}
	${CC} -c kernel/kabort.c -o build/kabort.o  ${CFLAGS}

link-i686: build
	${CC} -T kernel/arch/i586/linker.ld -o build/truthos.bin -ffreestanding -O2 -nostdlib build/boot.o build/kernel.o build/terminal.o build/gdt.o build/idt.o build/isr.o build/gdtc.o build/isrc.o build/idtc.o build/tmem.o build/kabort.o build/kassert.o build/kputs.o -lgcc

terminal: build
	${CC} -c kernel/terminal.c -o build/terminal.o ${CFLAGS}

gdt-i686: build
	${CC} -c kernel/gdt.c -o build/gdtc.o ${CFLAGS}
	${AS} kernel/arch/i586/gdt.s -o build/gdt.o

idt-i686: build
	${CC} -c kernel/idt.c -o build/idtc.o ${CFLAGS}
	${AS} kernel/arch/i586/idt.s -o build/idt.o

isr-i686: build
	${CC} -c kernel/isr.c -o build/isrc.o ${CFLAGS}
	${AS} kernel/arch/i586/isr.s -o build/isr.o

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

