MODULE_NAME := test
MODULE_OBJS := test.o

all: $(OUTFILE)

%.ko: $(MODULE_OBJS) ../link.ld
	mkdir -p $(shell dirname $@)
	echo $(OUTFILE)
	x86_64-elf-ld -shared -T ../link.ld $(MODULE_OBJS) -soname=$(MODULE_NAME)  -o $(OUTFILE)  -nostdlib -m elf_x86_64 -z max-page-size=0x1000


%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) -I ../../include
