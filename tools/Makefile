CC=gcc
BUILD_DIR := ../build
CFLAGS := -Wall -std=c11
LIBSODIUM_OBJS := ../kernel/crypto/ed25519/sign.c \
	../kernel/crypto/ed25519/keypair.c ../kernel/crypto/ed25519/open.c \
	../kernel/crypto/sha512/hash_sha512.c \
	../kernel/crypto/sha512/hash_sha512_cp.c \
	../kernel/crypto/truth_shims.c ../kernel/crypto/ed25519/verify.c \
	../kernel/crypto/ed25519/curve25519_ref10.c ./unix_shims_randombytes.c
VALGRIND := valgrind

.PHONY: all test

all: $(BUILD_DIR)/tools/truesign test

$(BUILD_DIR)/tools/truesign: truesign.c  $(LIBSODIUM_OBJS)
	mkdir -p $(shell dirname $@)
	$(CC) $(LIBSODIUM_OBJS) $< -o $@ $(CFLAGS) -I ../include/ -I ../kernel/libsodium

test: $(BUILD_DIR)/tools/truesign
	$(VALGRIND) $(BUILD_DIR)/tools/truesign generate \
		$(BUILD_DIR)/test_key.priv $(BUILD_DIR)/test_key.pub
	$(VALGRIND) $(BUILD_DIR)/tools/truesign sign $(BUILD_DIR)/test_key.priv \
		truesign.c $(BUILD_DIR)/truesign.c.signed
	$(VALGRIND) $(BUILD_DIR)/tools/truesign verify $(BUILD_DIR)/test_key.pub \
		$(BUILD_DIR)/truesign.c.signed
