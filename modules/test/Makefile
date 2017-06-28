MODULE_NAME := test
MODULE_OBJS := test.o
LD := x86_64-elf-ld
TRUESIGN := $(BUILD_DIR)/tools/truesign
TRUESIGN_PRIV_KEY := $(BUILD_DIR)/key.priv


all: $(OUTFILE)


%.so: $(MODULE_OBJS) ../link.ld
	mkdir -p $(shell dirname $@)
	echo $(BUILD_DIR)
	$(LD) -shared -T ../link.ld $(MODULE_OBJS) -soname=$(MODULE_NAME)  -o $@ -nostdlib -m elf_x86_64 -z max-page-size=0x1000


%.ko: %.so
	$(TRUESIGN) sign $(TRUESIGN_PRIV_KEY) $< $@


%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) -I ../../include
