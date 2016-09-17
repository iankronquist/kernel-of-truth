TRIPLE := x86_64-elf
KERNEL_MAJOR := 0
KERNEL_MINOR := 0
KERNEL_PATCH := 0
VCS_VERSION := $(shell git rev-parse HEAD)
WEBSITE := https://github.com/iankronquist/kernel-of-truth
MACROS := -D project_website=$(WEBSITE) -D kernel_major=$(KERNEL_MAJOR) -D kernel_minor=$(KERNEL_MINOR) -D kernel_patch=$(KERNEL_PATCH) -D vcs_version=$(VCS_VERSION)
CC := compiler/$(TRIPLE)/bin/$(TRIPLE)-gcc
CFLAGS := -std=gnu11 -MP -MMD -ffreestanding -fpic -O0 -Wall -Werror -Wextra -Wpedantic -g -I ./include $(MACROS)
AS := compiler/$(TRIPLE)/bin/$(TRIPLE)-gcc
ASFLAGS := -std=c11 -MP -MMD -ffreestanding -fpic -O0 -Wall -Werror -Wextra -Wpedantic -g -I ./include $(MACROS)
LD := compiler/$(TRIPLE)/bin/$(TRIPLE)-gcc
LDFLAGS := -nostdlib -ffreestanding -O0
ARCH := x64
QEMU := qemu-system-x86_64
QEMU_FLAGS := -no-reboot
OBJCOPY := objcopy
GRUB_MKRESCUE := grub-mkrescue
BOCHS := bochs

OBJ :=

COMPONENTS := kernel/arch/$(ARCH) kernel/core kernel/device

BUILD_DIR := build

KERNEL := $(BUILD_DIR)/truth.$(ARCH).elf

include $(patsubst %,%/component.$(ARCH).mk,$(COMPONENTS))


all: $(KERNEL)

$(KERNEL): $(KERNEL)64
	$(OBJCOPY) $< -O elf32-i386 $@

$(KERNEL)64: kernel/arch/$(ARCH)/link.ld $(OBJ)
	$(LD) -T kernel/arch/$(ARCH)/link.ld $(OBJ) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.device.$(ARCH).o: kernel/device/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.c.core.$(ARCH).o: kernel/core/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.c.arch.$(ARCH).o: kernel/arch/$(ARCH)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.S.$(ARCH).o: kernel/arch/$(ARCH)/%.S
	mkdir -p $(BUILD_DIR)
	$(AS) -c $< -o $@ $(ASFLAGS)

start: $(KERNEL)
	$(QEMU) -kernel $(KERNEL) $(QEMU_FLAGS) -serial file:$(BUILD_DIR)/qemu-serial.txt

start-log: $(KERNEL)
	$(QEMU) -kernel $(KERNEL) -d in_asm,cpu_reset,exec,int,guest_errors,pcall -D $(BUILD_DIR)/qemu.log $(QEMU_FLAGS)

start-debug:
	$(QEMU) -S -s -nographic -monitor stdio -kernel $(KERNEL) $(QEMU_FLAGS)

tags: $(OBJ)
	ctags -R kernel include

$(BUILD_DIR)/truth.iso: $(KERNEL) grub.cfg
	mkdir -p $(BUILD_DIR)/isodir/boot/grub
	cp $(KERNEL) $(BUILD_DIR)/isodir/boot/truth.elf
	cp grub.cfg $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	cd $(BUILD_DIR) && $(GRUB_MKRESCUE) -o truth.iso isodir

start-bochs: $(BUILD_DIR)/truth.iso
	$(BOCHS) -q -f bochsrc.txt

clean:
	rm -rf build/*

-include $(OBJ:.o=.d)

.PHONY: all clean start start-debug start-log
