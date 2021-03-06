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
MODULES :=
MODULE_CFLAGS := -std=c11 -MP -MMD -ffreestanding -O2 -Wall -Wextra \
	-fpic -nostdlib -I ../../include -D __C__ -mno-sse
include kernel/arch/$(ARCH)/Makefile
include kernel/core/Makefile
include kernel/crypto/Makefile
include kernel/device/Makefile
include modules/Makefile


PYTHON := python



CC := $(TRIPLE)-gcc
CFLAGS := -std=c11 -O2 -MP -MMD -mcmodel=kernel \
	-ffreestanding -fstack-protector-all \
	-Wall -Wextra \
	-I ./include $(MACROS) -D __C__ -mno-sse

AS := $(TRIPLE)-gcc
ASFLAGS := -O2 -MP -MMD -mcmodel=kernel \
	-ffreestanding \
	-Wall -Wextra \
	-I ./include $(MACROS) -D __ASM__

LD := $(TRIPLE)-gcc
LDFLAGS := -nostdlib -ffreestanding -O2 -mcmodel=kernel

NM := $(TRIPLE)-nm

MODULE_CC := $(CC)
MODULE_LD := $(TRIPLE)-ld
MODULE_AS := $(AS)


OBJCOPY := objcopy
GRUB_MKRESCUE := grub-mkrescue
STRIP := strip

TOOLS_CC := gcc

QEMU := qemu-system-x86_64
QEMU_FLAGS := -no-reboot -m 256M -serial file:$(BUILD_DIR)/serial.txt \
	-cpu Broadwell -initrd "$(strip $(MODULES))"

MAKE := make

.PHONY: all clean debug iso release start start-log tools

all: $(KERNEL) tools

tools: $(BUILD_DIR)/tools/truesign

$(BUILD_DIR)/tools/truesign:
	$(MAKE) -C tools/ CC=$(TOOLS_CC) ../$@

debug: CFLAGS += -g -fsanitize=undefined -D DEBUG
debug: ASFLAGS += -g -D DEBUG
debug: all

release: CFLAGS += -Werror
release: AFLAGS += -Werror
release: all
	$(STRIP) -s $(KERNEL)

$(KERNEL): $(KERNEL)64 $(MODULES)
	$(OBJCOPY) $< -O elf32-i386 $@

$(KERNEL)64: kernel/arch/$(ARCH)/link.ld $(BUILD_DIR)/symbols.o
	$(LD) -T kernel/arch/$(ARCH)/link.ld $(OBJ) $(BUILD_DIR)/symbols.o -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: kernel/%.c include/truth/key.h
	mkdir -p $(shell dirname $@)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.S.o: kernel/%.S
	mkdir -p $(shell dirname $@)
	$(AS) -c $< -o $@ $(ASFLAGS)

$(BUILD_DIR)/key.pub: $(BUILD_DIR)/tools/truesign
	$(BUILD_DIR)/tools/truesign generate $(BUILD_DIR)/key.priv $@

include/truth/key.h: $(BUILD_DIR)/key.pub
	$(BUILD_DIR)/tools/truesign header $(BUILD_DIR)/key.pub $@

$(BUILD_DIR)/symbols.o: $(OBJ) kernel/arch/$(ARCH)/link.ld
	$(LD) -T kernel/arch/$(ARCH)/link.ld $(OBJ) -o $(KERNEL)64 $(LDFLAGS)
	$(NM) $(KERNEL)64 | $(PYTHON) build_symbol_table.py $(BUILD_DIR)/symbols.S
	$(AS) -c $(BUILD_DIR)/symbols.S -o $@ $(ASFLAGS)

$(BUILD_DIR)/modules/%.mod: modules/% modules/link.ld $(BUILD_DIR)/key.pub
	mkdir -p $(shell dirname $@)
	$(MAKE) -C $< OUTFILE='../../$@' CFLAGS='$(MODULE_CFLAGS)' CC='$(MODULE_CC)' \
		BUILD_DIR='../../$(BUILD_DIR)' LD='$(MODULE_LD)' \
		AS='$(MODULE_AS)'

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

start-test: debug
	$(QEMU) -kernel $(KERNEL) $(QEMU_FLAGS) -monitor stdio -device isa-debug-exit,iobase=0xf4,iosize=0x04 || [ $$? -eq 1 ]


start-log: debug
	$(QEMU) -kernel $(KERNEL) -d in_asm,cpu_reset,exec,int,guest_errors,pcall \
		-D $(BUILD_DIR)/qemu.log $(QEMU_FLAGS) -monitor stdio

-include $(OBJ:.o=.d)
