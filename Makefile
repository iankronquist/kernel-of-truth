ARCH=i686-elf
TEST_CC=clang
GCOV=llvm-cov
GRUB_MKRESCUE=grub-mkrescue
CC=compiler/$(ARCH)/bin/$(ARCH)-gcc
LD=compiler/$(ARCH)/bin/$(ARCH)-gcc
AS=nasm
ASFLAGS=-felf32 -F dwarf -g
CFLAGS= -std=c11 -ffreestanding -O0 -Wall -Werror -Wextra -g -I ./include -I tlibc/include -D ARCH_X86
LDFLAGS=${CFLAGS} -nostdlib
TEST_CFLAGS= -std=c11 -O0 -Wall -Wextra -g -I ./include -coverage -Wno-format -D ARCH_USERLAND
QEMU_FLAGS= -m 1G -serial file:build/qemu-serial.log
VB=virtualbox
VBM=VBoxManage
TMP=/tmp

all: build/truthos.bin

tests: build/tests/kmem_tests build/tests/physical_allocator_tests

run-tests: tests
	./build/tests/kmem
	./build/tests/physical_allocator

build/boot.o:
	${AS} kernel/arch/x86/boot.s -o build/boot.o ${ASFLAGS}

build/libk.o: build/serial.o kernel/libk/*.c include/libk/*.h
	${CC} -c kernel/libk/kmem.c -o ${TMP}/kmem.o  ${CFLAGS}
	${CC} -c kernel/libk/kabort.c -o ${TMP}/kabort.o  ${CFLAGS}
	${CC} -c kernel/libk/kputs.c -o ${TMP}/kputs.o  ${CFLAGS}
	${CC} -c kernel/libk/klog.c -o ${TMP}/klog.o  ${CFLAGS}
	${CC} -c kernel/libk/physical_allocator.c -o ${TMP}/physical_allocator.o  ${CFLAGS}
	${LD} -r ${TMP}/kmem.o ${TMP}/kabort.o ${TMP}/kputs.o ${TMP}/klog.o ${TMP}/physical_allocator.o -o build/libk.o ${LDFLAGS}

build/processes.o: kernel/arch/x86/process.c kernel/arch/x86/process.s
	${CC} -c kernel/arch/x86/process.c -o ${TMP}/processesc.o ${CFLAGS}
	${AS} kernel/arch/x86/process.s -o ${TMP}/processess.o ${ASFLAGS}
	${LD} -r ${TMP}/processess.o ${TMP}/processesc.o -o build/processes.o ${LDFLAGS}

build/tests/kmem_tests: build kernel/libk/tests/stubs_tests.c kernel/libk/tests/kmem_tests.c
	${TEST_CC} kernel/libk/tests/stubs_tests.c kernel/libk/tests/kmem_tests.c  -o build/tests/kmem ${TEST_CFLAGS}

build/tests/physical_allocator_tests: build kernel/libk/tests/stubs_tests.c kernel/libk/tests/physical_allocator_tests.c
	${TEST_CC} kernel/libk/tests/stubs_tests.c kernel/libk/tests/physical_allocator_tests.c -o build/tests/physical_allocator ${TEST_CFLAGS}


build/io.o: kernel/arch/x86/io.s
	${AS} kernel/arch/x86/io.s -o build/io.o ${ASFLAGS}

build/kernel.o: build/libk.o build/terminal.o build/gdt.o build/idt.o build/tlibc.o build/keyboard.o build/timer.o build/paging.o build/io.o build/processes.o kernel/kernel.c build/lock.o
	${CC} -c kernel/kernel.c -o build/kernel.o  ${CFLAGS}

build/truthos.bin: build build/kernel.o build/boot.o kernel/arch/x86/linker.ld
	${CC} -T kernel/arch/x86/linker.ld -o build/truthos.bin -ffreestanding -O0 -nostdlib build/*.o

build/lock.o:
	${AS} kernel/arch/x86/lock.s -o build/lock.o ${ASFLAGS}

build/terminal.o: kernel/drivers/terminal.c
	${CC} -c kernel/drivers/terminal.c -o build/terminal.o ${CFLAGS}

build/timer.o: kernel/drivers/timer.s kernel/drivers/timer.c
	${AS} kernel/drivers/timer.s -o ${TMP}/timers.o ${ASFLAGS}
	${CC} -c kernel/drivers/timer.c -o ${TMP}/timerc.o ${CFLAGS}
	${LD} -r ${TMP}/timers.o ${TMP}/timerc.o -o build/timer.o ${LDFLAGS}

build/paging.o: kernel/arch/x86/paging.c kernel/arch/x86/paging.s
	${CC} -c kernel/arch/x86/paging.c -o ${TMP}/pagingc.o ${CFLAGS}
	${AS} kernel/arch/x86/paging.s -o ${TMP}/pagings.o ${ASFLAGS}
	${LD} -r ${TMP}/pagings.o ${TMP}/pagingc.o -o build/paging.o ${LDFLAGS}

build/idt.o: kernel/arch/x86/idt.c kernel/arch/x86/idt.s include/arch/x86/idt.h
	${CC} -c kernel/arch/x86/idt.c -o ${TMP}/idtc.o ${CFLAGS}
	${AS} kernel/arch/x86/idt.s -o ${TMP}/idts.o ${ASFLAGS}
	${LD} -r ${TMP}/idts.o ${TMP}/idtc.o -o build/idt.o ${LDFLAGS}

build/gdt.o: kernel/arch/x86/gdt.c kernel/arch/x86/gdt.s
	${CC} -c kernel/arch/x86/gdt.c -o ${TMP}/gdtc.o ${CFLAGS}
	${AS} kernel/arch/x86/gdt.s -o ${TMP}/gdts.o ${ASFLAGS}
	${LD} -r ${TMP}/gdts.o ${TMP}/gdtc.o -o build/gdt.o ${LDFLAGS}

build/serial.o: kernel/drivers/serial_port.c include/drivers/serial_port.h
	${CC} -c kernel/drivers/serial_port.c -o build/serial.o ${CFLAGS}

build/keyboard.o: kernel/drivers/keyboard.c kernel/drivers/keyboard.s
	${CC} -c kernel/drivers/keyboard.c -o /tmp/keyboard.o ${CFLAGS}
	${AS} kernel/drivers/keyboard.s -o ${TMP}/keyboards.o ${ASFLAGS}
	${LD} -r ${TMP}/keyboards.o ${TMP}/keyboard.o -o build/keyboard.o ${LDFLAGS}

build/tlibc.o: tlibc/string/string.c
	${CC} -c tlibc/string/string.c -o build/tlibc.o ${CFLAGS}

start:
	qemu-system-i386 -kernel build/truthos.bin ${QEMU_FLAGS}

start-log:
	qemu-system-i386 -kernel build/truthos.bin -d in_asm,cpu_reset,exec,int,guest_errors,pcall -no-reboot ${QEMU_FLAGS} &> qemu.log

start-debug:
	qemu-system-i386 -S -s -kernel build/truthos.bin ${QEMU_FLAGS} -curses

coverage: run-tests
	${GCOV} gcov *.gcno
	rm -f *.gcno
	rm -f *.gcda
	rm -f *tests.c.gcov
	rm -f *stubs.c.gcov

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
	cldoc generate -I ./include -DARCH_X86 -Wno-int-to-pointer-cast -- --output build/docs kernel/libk/*.c kernel/arch/x86/*.c kernel/drivers/*.c include/libk/*.h include/drivers/*.h kernel/*.c include/arch/x86/*.h --language c --report

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

.PHONY: all clean clean-all start run start-virtualbox iso start-debug start-log tests run-tests
