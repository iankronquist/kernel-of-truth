ARCH=i686-elf
TEST_CC=clang
GCOV=gcov
GRUB_MKRESCUE=grub2-mkrescue
CC=compiler/$(ARCH)/bin/$(ARCH)-gcc
AS=compiler/$(ARCH)/bin/$(ARCH)-as
ASFLAGS=-g
CFLAGS= -std=c11 -ffreestanding -O0 -Wall -Wextra -g -I ./include -I tlibc/include -D ARCH_X86
TEST_CFLAGS= -std=c11 -O0 -Wall -Wextra -g -I ./include -coverage -Wno-format -D ARCH_USERLAND
QEMU_FLAGS= -m 1G -serial file:build/qemu-serial.log
VB=virtualbox
VBM=VBoxManage

all: bootloader-x86 kernel link-x86

tests: build libk-tests

run-tests: tests
	./build/tests/kmem
	${GCOV} kmem.gcno
	./build/tests/physical_allocator
	${GCOV} physical_allocator.gcno

bootloader-x86: build
	${AS} kernel/arch/x86/boot.s -o build/boot.o ${ASFLAGS}

libk: build serial
	${CC} -c kernel/libk/kmem.c -o build/kmem.o  ${CFLAGS}
	${CC} -c kernel/libk/kabort.c -o build/kabort.o  ${CFLAGS}
	${CC} -c kernel/libk/kputs.c -o build/kputs.o  ${CFLAGS}
	${CC} -c kernel/libk/klog.c -o build/klog.o  ${CFLAGS}
	${CC} -c kernel/libk/physical_allocator.c -o build/physical_allocator.o  ${CFLAGS}

libk-tests:
	${TEST_CC} kernel/libk/tests/stubs.c kernel/libk/tests/kmem.c kernel/libk/kmem.c  -o build/tests/kmem ${TEST_CFLAGS}
	${TEST_CC} kernel/libk/tests/stubs.c kernel/libk/tests/physical_allocator.c -o build/tests/physical_allocator ${TEST_CFLAGS}

io: build
	${AS} -c kernel/arch/x86/io.s -o build/io.o ${ASFLAGS}

kernel: libk terminal gdt-x86 idt-x86 tlibc build keyboard timer paging-x86 io
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


serial: build
	${CC} -c kernel/drivers/serial_port.c -o build/serial_port.o ${CFLAGS}

keyboard: build
	${CC} -c kernel/drivers/keyboard.c -o build/keyboard.o ${CFLAGS}
	${AS} -c kernel/drivers/keyboard.s -o build/keyboards.o ${ASFLAGS}

tlibc: build
	${CC} -c tlibc/string/string.c -o build/tlibc.o ${CFLAGS}

start:
	qemu-system-i386 -kernel build/truthos.bin ${QEMU_FLAGS}

start-log:
	qemu-system-i386 -kernel build/truthos.bin -d in_asm,cpu_reset,exec,int,ioport,guest_errors,pcall -no-reboot ${QEMU_FLAGS} &> qemu.log

start-debug:
	qemu-system-i386 -S -s -kernel build/truthos.bin ${QEMU_FLAGS} -curses

build:
	mkdir build
	mkdir -p build/tests

clean:
	rm -rf build

clean-all: clean
	rm -f qemu.log
	rm -f *.gcno
	rm -f *.gcov
	rm -f *.gcda

docs:
	cldoc generate -I ./include -DARCH_X86 -- --output docs kernel/libk/*.c kernel/arch/x86/*.c kernel/drivers/*.c include/libk/*.h include/drivers/*.h kernel/*.c include/arch/x86/*.h --language c --report

run: all start
	rm -rf build

iso: all
	mkdir -p build/isodir/boot/grub
	cp build/truthos.bin build/isodir/boot/truthos.bin
	cp grub.cfg build/isodir/boot/grub/grub.cfg
	cd build && ${GRUB_MKRESCUE} -o truthos.iso isodir

start-virtualbox:
	-${VBM} unregistervm TruthOS --delete;
	echo "Create VM"
	${VBM} createvm --name TruthOS --register
	${VBM} modifyvm TruthOS --memory 1024
	${VBM} modifyvm TruthOS --vram 64
	${VBM} modifyvm TruthOS --nic1 nat
	${VBM} modifyvm TruthOS --nictype1 82540EM
	${VBM} modifyvm TruthOS --nictrace1 on
	${VBM} modifyvm TruthOS --nictracefile1 build/network.pcap
	${VBM} modifyvm TruthOS --uart1 0x3F8 4
	${VBM} modifyvm TruthOS --uartmode1 file build/virtualbox-serial.log
	${VBM} storagectl TruthOS --name "IDE Controller" --add ide
	${VBM} storageattach TruthOS --storagectl "IDE Controller" --port 0 \
	--device 0 --type dvddrive --medium build/truthos.iso
	echo "Run VM"
	${VB} --startvm TruthOS --dbg
