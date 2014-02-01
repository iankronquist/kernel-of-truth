
bootloader:
	../bin/bin/i586-elf-as boot.s -o boot.o
kernel: terminal
	../bin/bin/i586-elf-gcc -c kernel.c -o kernel.o -std=c99 -ffreestanding -O2 -Wall -Wextra
terminal:
	../bin/bin/i586-elf-gcc -c terminal.c -o terminal.o -std=c99 -ffreestanding -O2 -Wall -Wextra
link:
	../bin/bin/i586-elf-gcc -T linker.ld -o truthos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o terminal.o -lgcc	

all: bootloader kernel link 

start:
	qemu-system-i386 -kernel truthos.bin

clean:
	rm -rf *.o
	rm -rf truthos.bin

