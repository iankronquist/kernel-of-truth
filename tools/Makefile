CC=clang
BUILD_DIR := ../build
CFLAGS := -Wall -std=c11
LIBSODIUM_OBJS := ../kernel/crypto/ed25519/sign.c ../kernel/crypto/ed25519/keypair.c ../kernel/crypto/ed25519/open.c ../kernel/crypto/sha512/hash_sha512.c ../kernel/crypto/sha512/hash_sha512_cp.c ../kernel/crypto/truth_shims.c ../kernel/crypto/ed25519/verify.c ../kernel/crypto/ed25519/curve25519_ref10.c ./unix_shims_randombytes.c

all: $(BUILD_DIR)/tools/truesign

$(BUILD_DIR)/tools/truesign: truesign.c  $(LIBSODIUM_OBJS)
	mkdir -p $(shell dirname $@)
	$(CC) $(LIBSODIUM_OBJS) $< -o $@ $(CFLAGS) -I ../include/ -I ../kernel/libsodium
