TEST_CC=clang
CC=../bin/bin/i586-elf-gcc
AS=../bin/bin/i586-elf-as
CFLAGS= -std=c99 -ffreestanding -O0 -Wall -Wextra -g

all: bootloader-i586 kernel link-i586 

bootloader-i586: build
	${AS} kernel/arch/i586/boot.s -o build/boot.o

#kernel: terminal keyboard timer gdt-i586 idt-i586 isr-i586 irq-i586 tmem build
kernel: kernel-functions terminal tmem build
	${CC} -c kernel/kernel.c -o build/kernel.o  ${CFLAGS}

kernel-functions: build
	${CC} -c kernel/kernel-functions/kassert.c -o build/kassert.o  ${CFLAGS}
	${CC} -c kernel/kernel-functions/kputs.c -o build/kputs.o  ${CFLAGS}
	${CC} -c kernel/kernel-functions/kabort.c -o build/kabort.o  ${CFLAGS}

link-i586: build
	${CC} -T kernel/arch/i586/linker.ld -o build/truthos.bin -ffreestanding -O2 -nostdlib build/*.o -lgcc

terminal: build
	${CC} -c kernel/terminal.c -o build/terminal.o ${CFLAGS}

gdt-i586: build
	${CC} -c kernel/arch/i586/gdt.c -o build/gdtc.o ${CFLAGS}
	#${AS} kernel/arch/i586/gdt.s -o build/gdt.o

idt-i586: build
	${CC} -c kernel/arch/i586/idt.c -o build/idtc.o ${CFLAGS}
	${AS} kernel/arch/i586/idt.s -o build/idt.o

isr-i586: build
	${CC} -c kernel/arch/i586/isr.c -o build/isrc.o ${CFLAGS}
	${AS} kernel/arch/i586/isr.s -o build/isr.o

irq-i586: io-i586 build
	${CC} -c kernel/arch/i586/irq.c -o build/irqc.o ${CFLAGS}
	${AS} kernel/arch/i586/irq.s -o build/irq.o

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

