# Target information
ARCH := x64
TRIPLE := x86_64-elf

# Version & meta data
KERNEL_MAJOR := 0
KERNEL_MINOR := 0
KERNEL_PATCH := 0
VCS_VERSION := $(shell git rev-parse HEAD)
WEBSITE := https://github.com/iankronquist/kernel-of-truth
MACROS := -dD -D project_website='"$(WEBSITE)"' -D kernel_major=$(KERNEL_MAJOR) -D kernel_minor=$(KERNEL_MINOR) -D kernel_patch=$(KERNEL_PATCH) -D vcs_version='"$(VCS_VERSION)"'

# Build tools & flags
CC := compiler/$(TRIPLE)/bin/$(TRIPLE)-gcc
CFLAGS := -std=c11 -MP -MMD -ffreestanding -O2 -Wall -Wextra -I ./include $(MACROS) -D __C__
AS := compiler/$(TRIPLE)/bin/$(TRIPLE)-gcc
ASFLAGS := -std=c11 -MP -MMD -ffreestanding -fpic -O2 -Wall -Wextra -I ./include $(MACROS) -D __ASM__
LD := compiler/$(TRIPLE)/bin/$(TRIPLE)-gcc
LDFLAGS := -nostdlib -ffreestanding -O2
OBJCOPY := objcopy
GRUB_MKRESCUE := grub-mkrescue


# Emulators & flags
QEMU := qemu-system-x86_64
QEMU_FLAGS := -no-reboot -m 256M
BOCHS := bochs

COMPONENTS := kernel/arch/$(ARCH) kernel/core kernel/device

BUILD_DIR := build/$(ARCH)

OBJ :=
FULL_OBJ = $(patsubst %,$(BUILD_DIR)/%,$(OBJ))

KERNEL := $(BUILD_DIR)/truth.$(ARCH).elf

include $(patsubst %,%/component.$(ARCH).mk,$(COMPONENTS))


all: $(KERNEL)

debug: CFLAGS += -g -fsanitize=undefined
debug: ASFLAGS += -g
debug: all

release: CFLAGS += -Werror
release: AFLAGS += -Werror
release: all
	strip --strip-all $(KERNEL)

$(KERNEL): $(KERNEL)64
	$(OBJCOPY) $< -O elf32-i386 $@

$(KERNEL)64: kernel/arch/$(ARCH)/link.ld $(FULL_OBJ)
	$(LD) -T kernel/arch/$(ARCH)/link.ld $(FULL_OBJ) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: kernel/*/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.c.o: kernel/arch/$(ARCH)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.S.o: kernel/arch/$(ARCH)/%.S
	mkdir -p $(BUILD_DIR)
	$(AS) -c $< -o $@ $(ASFLAGS)

docs: $(BUILD_DIR)/docs/index.html

$(BUILD_DIR)/docs/index.html: include/truth/*.h include/arch/*/*.h kernel/*/*.c kernel/arch/*/*.c
	cldoc generate -D __C__ -I ./include -Wno-pragma-once-outside-header -ffreestanding $(MACROS) -- --output $(BUILD_DIR)/docs include/truth/*.h include/arch/*/*.h kernel/*/*.c kernel/arch/*/*.c --language c --report

start: debug
	$(QEMU) -kernel $(KERNEL) $(QEMU_FLAGS) -monitor stdio -serial file:$(BUILD_DIR)/qemu-serial.txt

start-log: $(KERNEL)
	$(QEMU) -kernel $(KERNEL) -d in_asm,cpu_reset,exec,int,guest_errors,pcall -D $(BUILD_DIR)/qemu.log $(QEMU_FLAGS)

start-debug:
	$(QEMU) -S -s -monitor stdio -kernel $(KERNEL) $(QEMU_FLAGS)

tags: kernel/arch/$(ARCH)/*.c kernel/*/*.c kernel/arch/$(ARCH)/*.S include/*/*.h
	ctags -R kernel include

iso: $(BUILD_DIR)/truth.iso

$(BUILD_DIR)/truth.iso: $(KERNEL) grub.cfg
	mkdir -p $(BUILD_DIR)/isodir/boot/grub
	cp $(KERNEL) $(BUILD_DIR)/isodir/boot/truth.elf
	cp grub.cfg $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	cd $(BUILD_DIR) && $(GRUB_MKRESCUE) -o truth.iso isodir

start-bochs: $(BUILD_DIR)/truth.iso
	$(BOCHS) -q -f bochsrc.txt

clean:
	rm -rf build/*

-include $(FULL_OBJ:.o=.d)

.PHONY: all clean debug docs iso release start start-debug start-log
