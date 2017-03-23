CC=clang
BUILD_DIR := ../build
CFLAGS := -Wall -std=c11
LIBSODIUM_OBJS := ../kernel/libsodium/ref10/sign.c ../kernel/libsodium/ref10/keypair.c ../kernel/libsodium/ref10/open.c ../kernel/libsodium/crypto_hash.c ../kernel/libsodium/crypto_sign.c ../kernel/libsodium/curve25519_ref10.c ../kernel/libsodium/hash_sha512.c ../kernel/libsodium/hash_sha512_cp.c ../kernel/libsodium/sign_ed25519.c ../kernel/libsodium/truth_shims.c ../kernel/libsodium/verify.c

$(BUILD_DIR)/tools/truesign: truesign.c  $(LIBSODIUM_OBJS)
	mkdir -p $(shell dirname $@)
	$(CC) $^ -o $@ $(CFLAGS) -I ../include/ -I ../kernel/libsodium

#clang tools/signtool.c -I include/ kernel/libsodium/ref10/sign.c kernel/libsodium/ref10/keypair.c kernel/libsodium/ref10/open.c kernel/libsodium/crypto_hash.c kernel/libsodium/crypto_sign.c kernel/libsodium/curve25519_ref10.c kernel/libsodium/hash_sha512.c kernel/libsodium/hash_sha512_cp.c kernel/libsodium/sign_ed25519.c kernel/libsodium/truth_shims.c kernel/libsodium/verify.c -I kernel/libsodium/ -o signtool
