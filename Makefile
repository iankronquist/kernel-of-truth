# The name of the target platform.
ARCH := x86

# The place to put all of the build artifacts.
BUILD_DIR := build

# Flags for the tools.
CFLAGS := -std=c11 -MP -MMD -ffreestanding -O0 -Wall -Werror -Wextra -g -I ./include -I tlibc/include
LDFLAGS := $(CFLAGS) -nostdlib
QEMU_FLAGS := -m 1G
TEST_CFLAGS= -std=c11 -O0 -Wall -Wextra -g -I ./include -coverage -Wno-format -D ARCH_USERLAND


# Select the appropriate architecture specific CFLAGS and QEMU
ifeq ($(ARCH),x86)
QEMU := qemu-system-i386
QEMU_FLAGS += -serial file:$(BUILD_DIR)/qemu-serial.log
TARGET := i686-elf
# The path to GCC and friends.
TOOLCHAIN := compiler/$(TARGET)/bin/$(TARGET)
AS := yasm
ASFLAGS := -felf -g DWARF2
else ifeq ($(ARCH),arm)
CFLAGS += -mcpu=arm1176jzf-s -fpic
QEMU := qemu-system-arm
QEMU_FLAGS += -serial stdio -M raspi2
TARGET := arm-none-eabi
# The path to GCC and friends.
TOOLCHAIN := compiler/$(TARGET)/bin/$(TARGET)
AS := $(TOOLCHAIN)-gcc
ASFLAGS := -c $(CFLAGS)
endif

# Tools.
CC := $(TOOLCHAIN)-gcc
GRUB_MKRESCUE := grub-mkrescue
VB=virtualbox
VBM=VBoxManage
TEST_CC=clang
GCOV=llvm-cov

# The kernel is broken down into several components.
COMPONENTS := kernel/arch/$(ARCH) kernel kernel/libk tlibc kernel/drivers

# The name of the final elf file being built.
KERNEL := $(BUILD_DIR)/truth.$(ARCH).elf

# The list of object files to build.
OBJ :=

# Always ensure that the $(BUILD_DIR) directory exists.
# This is a bit of a hack.
$(shell mkdir -p $(BUILD_DIR)/tests)

# Include module make include files.
# Each component has a list of object files which must be built.
include $(patsubst %,%/component.mk,$(COMPONENTS))

# The default target is to $(BUILD_DIR) the kernel elf file.
all: $(KERNEL)

$(KERNEL): kernel/arch/$(ARCH)/linker.ld $(OBJ)
	$(CC) -T kernel/arch/$(ARCH)/linker.ld $(OBJ) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: kernel/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.libk.o: kernel/libk/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.$(ARCH).o: kernel/arch/$(ARCH)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.asm.$(ARCH).o: kernel/arch/$(ARCH)/%.asm
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/%.S.$(ARCH).o: kernel/arch/$(ARCH)/%.S
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/%.driver.o: kernel/drivers/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.tlibc.o: tlibc/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/truth.iso: $(KERNEL) grub.cfg
	mkdir -p $(BUILD_DIR)/isodir/boot/grub
	cp $(KERNEL) $(BUILD_DIR)/isodir/boot/truth.elf
	cp grub.cfg $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	cd $(BUILD_DIR) && $(GRUB_MKRESCUE) -o truth.iso isodir

clean:
	rm -rf $(BUILD_DIR)/*

start: $(KERNEL)
	$(QEMU) -kernel $(KERNEL) $(QEMU_FLAGS)

start-log:
	$(QEMU) -kernel $(KERNEL) -d in_asm,cpu_reset,exec,int,guest_errors,pcall -no-reboot -D $(BUILD_DIR)/qemu.log $(QEMU_FLAGS)

start-debug:
	$(QEMU) -S -s -curses -kernel $(KERNEL) $(QEMU_FLAGS)

start-virtualbox:
	-$(VBM) unregistervm TruthOS --delete;
	$(VBM) createvm --name TruthOS --register
	$(VBM) modifyvm TruthOS --memory 1024
	$(VBM) modifyvm TruthOS --vram 64
	$(VBM) modifyvm TruthOS --nic1 nat
	$(VBM) modifyvm TruthOS --nictype1 82540EM
	$(VBM) modifyvm TruthOS --nictrace1 on
	$(VBM) modifyvm TruthOS --nictracefile1 $(BUILD_DIR)/network.pcap
	$(VBM) modifyvm TruthOS --uart1 0x3F8 4
	$(VBM) modifyvm TruthOS --uartmode1 file $(BUILD_DIR)/virtualbox-serial.log
	$(VBM) storagectl TruthOS --name "IDE Controller" --add ide
	$(VBM) storageattach TruthOS --storagectl "IDE Controller" --port 0 \
	--device 0 --type dvddrive --medium $(BUILD_DIR)/truthos.iso
	$(VB) --startvm TruthOS --dbg

docs:
	cldoc generate -I ./include  -Wno-int-to-pointer-cast -- --output build/docs kernel/libk/*.c kernel/arch/x86/*.c kernel/drivers/*.c include/truth/*.h include/drivers/*.h kernel/*.c include/arch/x86/*.h --language c --report

tests: build/tests/kmem_tests build/tests/physical_allocator_tests docs-tests test_status_types

# Check that documentation coverage didn't change.
# We don't care about enum values.
docs-tests: docs
	grep -E 'name="enum"\s+undocumented="0"' build/docs/xml/report.xml
	grep -E 'name="typedef"\s+undocumented="0"' build/docs/xml/report.xml
	grep -E 'name="struct"\s+undocumented="0"' build/docs/xml/report.xml
	grep -E 'name="function"\s+undocumented="0"' build/docs/xml/report.xml

test_status_types:
	# Check that function declarations or definitions which return status_t
	# are decorated with the checked attribute.
	! grep -RE 'status_t \w*\(.*\)' include/ kernel/

run-tests: tests
	$(BUILD_DIR)/tests/kmem
	$(BUILD_DIR)/tests/physical_allocator

build/tests/kmem_tests: build kernel/libk/tests/stubs_tests.c kernel/libk/tests/kmem_tests.c
	$(TEST_CC) kernel/libk/tests/stubs_tests.c kernel/libk/tests/kmem_tests.c  -o $(BUILD_DIR)/tests/kmem $(TEST_CFLAGS)

build/tests/physical_allocator_tests: build kernel/libk/tests/stubs_tests.c kernel/libk/tests/physical_allocator_tests.c
	$(TEST_CC) kernel/libk/tests/stubs_tests.c kernel/libk/tests/physical_allocator_tests.c -o $(BUILD_DIR)/tests/physical_allocator $(TEST_CFLAGS)

coverage: run-tests
	$(GCOV) gcov *.gcno
	rm -f *.gcno
	rm -f *.gcda
	rm -f *tests.c.gcov
	rm -f *stubs.c.gcov



-include $(OBJ:.o=.d)

.PHONY: all clean start start-log start-debug start-virtualbox run-tests tests test_status_types docs docs-tests
