TRIPLE := x86_64-elf
ARCH := x64

KERNEL_MAJOR := 0
KERNEL_MINOR := 0
KERNEL_PATCH := 0
VCS_VERSION := $(shell git rev-parse HEAD)
WEBSITE := https://github.com/iankronquist/kernel-of-truth
MACROS := -dD \
	-D project_website='"$(WEBSITE)"' \
	-D kernel_major='"$(KERNEL_MAJOR)"' \
    -D kernel_minor='"$(KERNEL_MINOR)"' \
    -D kernel_patch='"$(KERNEL_PATCH)"' \
	-D vcs_version='"$(VCS_VERSION)"'

BUILD_DIR := build

KERNEL := $(BUILD_DIR)/truth.$(ARCH).elf

OBJ :=
include kernel/arch/$(ARCH)/Makefile
include kernel/core/Makefile
include kernel/device/Makefile
include kernel/libsodium/Makefile


CC := $(TRIPLE)-gcc
CFLAGS := -std=c11 -MP -MMD -ffreestanding -O2 -Wall -Wextra \
	-I ./include $(MACROS) -D __C__ -mcmodel=large -I kernel/libsodium

AS := $(TRIPLE)-gcc
ASFLAGS := -MP -MMD -ffreestanding -O2 -Wall -Wextra \
	-I ./include $(MACROS) -D __ASM__

LD := $(TRIPLE)-gcc
LDFLAGS := -nostdlib -ffreestanding -O2

OBJCOPY := objcopy
GRUB_MKRESCUE := grub-mkrescue
STRIP := strip

QEMU := qemu-system-x86_64
QEMU_FLAGS := -no-reboot -m 256M -serial file:$(BUILD_DIR)/serial.txt \
	-cpu Broadwell

.PHONY: all clean debug iso release start start-log

all: $(KERNEL)

debug: CFLAGS += -g -fsanitize=undefined
debug: ASFLAGS += -g
debug: all

release: CFLAGS += -Werror
release: AFLAGS += -Werror
release: all
	$(STRIP) -s $(KERNEL)

$(KERNEL): $(KERNEL)64
	$(OBJCOPY) $< -O elf32-i386 $@

$(KERNEL)64: kernel/arch/$(ARCH)/link.ld $(OBJ)
	$(LD) -T kernel/arch/$(ARCH)/link.ld $(OBJ) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: kernel/%.c
	mkdir -p $(shell dirname $@)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.S.o: kernel/%.S
	mkdir -p $(shell dirname $@)
	$(AS) -c $< -o $@ $(ASFLAGS)

tags: kernel/arch/$(ARCH)/*.c kernel/core/*.c kernel/device/*.c \
		include/arch/$(ARCH)/*.h include/truth/*.h
	ctags -R kernel include

iso: $(BUILD_DIR)/truth.iso

$(BUILD_DIR)/truth.iso: $(KERNEL) grub.cfg
	mkdir -p $(BUILD_DIR)/isodir/boot/grub
	cp $(KERNEL) $(BUILD_DIR)/isodir/boot/truth.elf
	cp grub.cfg $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	cd $(BUILD_DIR) && $(GRUB_MKRESCUE) -o truth.iso isodir

clean:
	rm -rf $(BUILD_DIR)

start: debug
	$(QEMU) -kernel $(KERNEL) $(QEMU_FLAGS) -monitor stdio

start-log: $(KERNEL)
	$(QEMU) -kernel $(KERNEL) -d in_asm,cpu_reset,exec,int,guest_errors,pcall \
		-D $(BUILD_DIR)/qemu.log $(QEMU_FLAGS) -monitor stdio

-include $(OBJ:.o=.d)
