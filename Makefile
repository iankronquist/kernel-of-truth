ARCH=i686-elf
TEST_CC=clang
CC=compiler/$(ARCH)/bin/$(ARCH)-gcc
AS=compiler/$(ARCH)/bin/$(ARCH)-as
ASFLAGS=-g
CFLAGS= -std=c11 -ffreestanding -O0 -Wall -Wextra -g -I ./include -I tlibc/include
TEST_CFLAGS= -std=c11 -O0 -Wall -Wextra -g -I ./include
QEMU_FLAGS= -m 1G

all: bootloader-x86 kernel link-x86 

tests: build libk-tests

bootloader-x86: build
	${AS} kernel/arch/x86/boot.s -o build/boot.o ${ASFLAGS}

libk: build
	${CC} -c kernel/libk/kmem.c -o build/kmem.o  ${CFLAGS}
	${CC} -c kernel/libk/kabort.c -o build/kabort.o  ${CFLAGS}
	${CC} -c kernel/libk/kputs.c -o build/kputs.o  ${CFLAGS}
	${CC} -c kernel/libk/physical_allocator.c -o build/physical_allocator.o  ${CFLAGS}

libk-tests:
	${TEST_CC} kernel/libk/tests/stubs.c kernel/libk/tests/kmem.c kernel/libk/kmem.c  -o build/tests/kmem ${TEST_CFLAGS}
	${TEST_CC} kernel/libk/tests/stubs.c kernel/libk/tests/physical_allocator.c -o build/tests/physical_allocator ${TEST_CFLAGS}

kernel: libk terminal gdt-x86 idt-x86 tlibc build keyboard timer paging-x86
	${CC} -c kernel/kernel.c -o build/kernel.o  ${CFLAGS}

link-x86: build
	${CC} -T kernel/arch/x86/linker.ld -o build/truthos.bin -ffreestanding -O0 -nostdlib build/*.o -lgcc

terminal: build
	${CC} -c kernel/drivers/terminal.c -o build/terminal.o ${CFLAGS}

timer: build
	${AS} -c kernel/drivers/timer.s -o build/timers.o ${ASFLAGS}
	${CC} -c kernel/drivers/timer.c -o build/timerc.o ${CFLAGS}

paging-x86: build
	${AS} -c kernel/arch/x86/paging.s -o build/pagings.o ${ASFLAGS}
	${CC} -c kernel/arch/x86/paging.c -o build/pagingc.o ${CFLAGS}


gdt-x86: build
	${CC} -c kernel/arch/x86/gdt.c -o build/gdtc.o ${CFLAGS}
	${AS} kernel/arch/x86/gdt.s -o build/gdts.o ${ASFLAGS}

idt-x86: build keyboard
	${CC} -c kernel/arch/x86/idt.c build/keyboard.o -o build/idtc.o ${CFLAGS}
	${AS} kernel/arch/x86/idt.s -o build/idts.o ${ASFLAGS}

keyboard: build
	${CC} -c kernel/drivers/keyboard.c -o build/keyboard.o ${CFLAGS}

tlibc: build
	${CC} -c tlibc/string/string.c -o build/tlibc.o ${CFLAGS}

start:
	qemu-system-i386 -kernel build/truthos.bin ${QEMU_FLAGS}

start-log:
	qemu-system-i386 -kernel build/truthos.bin -d in_asm,cpu_reset,exec,int,ioport,guest_errors,pcall -no-reboot ${QEMU_FLAGS} &> qemu.log

start-debug:
	qemu-system-i386 -S -s -kernel build/truthos.bin ${QEMU_FLAGS}

build:
	mkdir build
	mkdir -p build/tests

clean:
	rm -rf build

run: all start
	rm -rf build

