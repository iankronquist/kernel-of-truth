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
KERNEL64 := $(BUILD_DIR)/truth.$(ARCH).elf64
LOADER := $(BUILD_DIR)/truth_loader.$(ARCH).elf
LOADER64 := $(BUILD_DIR)/truth_loader.$(ARCH).elf64

OBJ :=
LOADER_OBJS :=
MODULES :=
MODULE_CFLAGS := -std=c11 -MP -MMD -ffreestanding -O2 -Wall -Wextra \
	-fpic -nostdlib -I ../../include -D __C__ -mno-sse
include loader/$(ARCH)/Makefile
include kernel/arch/$(ARCH)/Makefile
include kernel/core/Makefile
include kernel/crypto/Makefile
include kernel/device/Makefile
include modules/Makefile


PYTHON := python
OD := od

LOADER_FLAGS := -O2 -MP -MMD \
	-ffreestanding \
	-Wall -Wextra \
	-I ./include -mno-sse

RANOM_NUMBER := $(strip $(shell $(OD) -vAn -N8 -tu8 < /dev/urandom))
LOADER_CFLAGS := $(LOADER_FLAGS) -D __C__ -std=c11 -D Boot_Compile_Random_Number=$(RANOM_NUMBER)ul
LOADER_ASFLAGS := $(LOADER_FLAGS) -D __ASM__


KERNEL_FLAGS := -O2 -MP -MMD -mno-sse -Wall -Wextra -ffreestanding -I ./include -fPIC
CC := $(TRIPLE)-gcc
CFLAGS := -std=c11 $(KERNEL_FLAGS) $(MACROS) -D __C__

AS := $(TRIPLE)-gcc
ASFLAGS := $(KERNEL_FLAGS) $(MACROS) -D __ASM__

LD := $(TRIPLE)-ld
LDFLAGS := -nostdlib -O2 -soname="Kernel of Truth" -m elf_x86_64 -z max-page-size=0x1000 -fPIE -fPIC

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

all: $(LOADER) $(MODULES)


$(LOADER64): loader/$(ARCH)/link.ld $(LOADER_OBJS) $(KERNEL64)
	$(CC) -T loader/$(ARCH)/link.ld $(LOADER_OBJS) $(LOADER_CFLAGS) -o $@ $< -nostdlib

$(LOADER): $(LOADER64)
	$(OBJCOPY) $< -O elf32-i386 $@

tools: $(BUILD_DIR)/tools/truesign

$(BUILD_DIR)/tools/truesign:
	$(MAKE) -C tools/ CC=$(TOOLS_CC) ../$@

debug: CFLAGS += -g -fsanitize=undefined
debug: ASFLAGS += -g
debug: all

release: LOADER_CFLAGS += -Werror
release: LOADER_ASFLAGS += -Werror
release: CFLAGS += -Werror
release: AFLAGS += -Werror
release: all
	$(STRIP) -s $(KERNEL)

$(KERNEL): $(KERNEL)64 $(MODULES)
	$(OBJCOPY) $< -O elf32-i386 $@

$(KERNEL64): kernel/arch/$(ARCH)/link.ld $(OBJ)
	$(LD) -T kernel/arch/$(ARCH)/link.ld -o $@ $(OBJ) -shared -soname="truth" -ffreestanding -nostdlib

$(BUILD_DIR)/%.c.o: kernel/%.c include/truth/key.h
	mkdir -p $(shell dirname $@)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.S.o: kernel/%.S
	mkdir -p $(shell dirname $@)
	$(AS) -c $< -o $@ $(ASFLAGS)

$(BUILD_DIR)/loader/%.S.o: loader/%.S
	mkdir -p $(shell dirname $@)
	$(AS) -c $< -o $@ $(LOADER_ASFLAGS)

$(BUILD_DIR)/loader/%.c.o: loader/%.c include/loader/kernel.h
	mkdir -p $(shell dirname $@)
	$(CC) -c $< -o $@ $(LOADER_CFLAGS)


$(BUILD_DIR)/key.pub: $(BUILD_DIR)/tools/truesign
	$(BUILD_DIR)/tools/truesign generate $(BUILD_DIR)/key.priv $@

include/truth/key.h: $(BUILD_DIR)/key.pub
	$(BUILD_DIR)/tools/truesign header $(BUILD_DIR)/key.pub $@

$(BUILD_DIR)/modules/%.ko: modules/% modules/link.ld $(BUILD_DIR)/key.pub
	mkdir -p $(shell dirname $@)
	$(MAKE) -C $< OUTFILE='../../$@' CFLAGS='$(MODULE_CFLAGS)' CC='$(MODULE_CC)' \
		BUILD_DIR='../../$(BUILD_DIR)' LD='$(MODULE_LD)' \
		AS='$(MODULE_AS)'

tags: kernel/arch/$(ARCH)/*.c kernel/core/*.c kernel/device/*.c include/arch/$(ARCH)/*.h include/truth/*.h loader/$(ARCH)/*.c
	ctags -R kernel include loader

iso: $(BUILD_DIR)/truth.iso

$(BUILD_DIR)/truth.iso: $(KERNEL) grub.cfg
	mkdir -p $(BUILD_DIR)/isodir/boot/grub
	cp $(KERNEL) $(BUILD_DIR)/isodir/boot/truth.elf
	cp grub.cfg $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	cd $(BUILD_DIR) && $(GRUB_MKRESCUE) -o truth.iso isodir

clean:
	rm -rf $(BUILD_DIR)

start: debug
	$(QEMU) -kernel $(LOADER) $(QEMU_FLAGS) -monitor stdio

start-log: debug
	$(QEMU) -kernel $(LOADER) -d in_asm,cpu_reset,exec,int,guest_errors,pcall \
		-D $(BUILD_DIR)/qemu.log $(QEMU_FLAGS) -monitor stdio

-include $(OBJ:.o=.d)
