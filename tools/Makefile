CC=gcc
BUILD_DIR := ../build
CFLAGS := -Wall -Wextra -Werror -std=c11 -O2
KERNEL_ROOT := ..
LIBSODIUM_OBJS := $(KERNEL_ROOT)/kernel/crypto/ed25519/sign.c \
	$(KERNEL_ROOT)/kernel/crypto/ed25519/keypair.c $(KERNEL_ROOT)/kernel/crypto/ed25519/open.c \
	$(KERNEL_ROOT)/kernel/crypto/sha512/hash_sha512.c \
	$(KERNEL_ROOT)/kernel/crypto/sha512/hash_sha512_cp.c \
	$(KERNEL_ROOT)/kernel/crypto/truth_shims.c $(KERNEL_ROOT)/kernel/crypto/ed25519/verify.c \
	$(KERNEL_ROOT)/kernel/crypto/ed25519/curve25519_ref10.c $(KERNEL_ROOT)/tools/unix_shims_randombytes.c
VALGRIND := valgrind

.PHONY: all test

all: $(BUILD_DIR)/tools/truesign test

$(BUILD_DIR)/tools/truesign: truesign.c $(LIBSODIUM_OBJS)
	mkdir -p $(shell dirname $@)
	$(CC) $(LIBSODIUM_OBJS) $< -o $@ $(CFLAGS) -I ../include/

test: $(BUILD_DIR)/tools/truesign
	$(VALGRIND) $(BUILD_DIR)/tools/truesign generate \
		$(BUILD_DIR)/test_key.priv $(BUILD_DIR)/test_key.pub
	$(VALGRIND) $(BUILD_DIR)/tools/truesign sign $(BUILD_DIR)/test_key.priv \
		truesign.c $(BUILD_DIR)/truesign.c.signed
	$(VALGRIND) $(BUILD_DIR)/tools/truesign verify $(BUILD_DIR)/test_key.pub \
		$(BUILD_DIR)/truesign.c.signed
	$(VALGRIND) $(BUILD_DIR)/tools/truesign header $(BUILD_DIR)/test_key.pub \
		$(BUILD_DIR)/test_key.h
