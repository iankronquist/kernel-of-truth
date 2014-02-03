
CC=../bin/bin/i586-elf-gcc
AS=../bin/bin/i586-elf-as

all: bootloader-i586 kernel link-i586 

bootloader-i586: build
	${AS} kernel/arch/i586/boot.s -o build/boot.o
	${AS} kernel/arch/i586/gdt.s -o build/gdt.o
kernel: terminal build
	${CC} -c kernel/kernel.c -o build/kernel.o -std=c99 -ffreestanding -O2 -Wall -Wextra
terminal: build
	${CC} -c kernel/terminal.c -o build/terminal.o -std=c99 -ffreestanding -O2 -Wall -Wextra
link-i586: build
	${CC} -T kernel/arch/i586/linker.ld -o build/truthos.bin -ffreestanding -O2 -nostdlib build/boot.o build/kernel.o build/terminal.o build/gdt.o -lgcc	

start:
	qemu-system-i386 -kernel build/truthos.bin

build:
	mkdir build

clean:
	rm -rf build

