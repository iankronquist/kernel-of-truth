#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <truth/crypto.h>

#define SIGNATURE_MAGIC_STRING "Kernel of Truth truesign 1.0.0"

int usage(int argc, char *argv[]) {
    fprintf(stderr,
            "Usage: %s <command> <args>\n"
            "Commands:\n"
            "\tgenerate privkey pubkey      \tGenerate an ed25519 key pair\n"
            "\tsign privkey in_file out_file\tSign file with public key\n"
            "\tverify pubkey file           \tVerify file with private key\n",
            argv[0]);
    return EXIT_FAILURE;
}

int verify_file(char *public_key_name, char *file_name) {
    int error = 0;
    crypto_hash_sha512_state state;
    unsigned char signature[crypto_hash_sha512_BYTES];
    unsigned char public_key[crypto_sign_PUBLICKEYBYTES];
    unsigned char hash[crypto_hash_sha512_BYTES];
    size_t read;
    size_t test_file_size;
    size_t signature_field_size = crypto_sign_BYTES +
        sizeof(SIGNATURE_MAGIC_STRING);
    size_t test_file_data_size;
    size_t test_file_data_remainder_size;
    unsigned char file_contents[512];
    FILE *public_key_file = NULL;
    FILE *test_file = NULL;

    public_key_file = fopen(public_key_name, "r");
    if (public_key_file == NULL) {
        perror("opening public key file");
        error = -1;
        goto out;
    }

    test_file = fopen(file_name, "r");
    if (test_file == NULL) {
        perror("opening test file");
        error = -1;
        goto out;
    }

    fseek(test_file, 0, SEEK_END);

    test_file_size = ftell(test_file);
    if (test_file_size <= signature_field_size) {
        fprintf(stderr, "File too small to have signature\n");
        error = -1;
        goto out;
    }
    rewind(test_file);

    test_file_data_size = (test_file_size - signature_field_size) /
        sizeof(file_contents);
    test_file_data_remainder_size = (test_file_size - signature_field_size) %
        sizeof(file_contents);

    crypto_hash_sha512_init(&state);

    for (size_t i = 0; i < test_file_data_size; ++i) {
        read = fread(file_contents, 1, sizeof(file_contents), test_file);
        if (ferror(test_file) != 0) {
            perror("Reading test file");
            error = -1;
            goto out;
        }

        error = crypto_hash_sha512_update(&state, file_contents, read);
        if (error != 0) {
            fprintf(stderr, "Updating hash\n");
            error = -1;
            goto out;
        }
    }

    if (test_file_data_remainder_size != 0) {
        read = fread(file_contents, 1, test_file_data_remainder_size, test_file);
        if (ferror(test_file) != 0) {
            perror("Reading test file");
            error = -1;
            goto out;
        }

        error = crypto_hash_sha512_update(&state, file_contents, read);
        if (error != 0) {
            fprintf(stderr, "Updating hash\n");
            error = -1;
            goto out;
        }
    }

    error = crypto_hash_sha512_final(&state, hash);
    if (error != 0) {
        fprintf(stderr, "Finalizing hash\n");
        error = -1;
        goto out;
    }

    error = fseek(test_file, sizeof(signature), SEEK_END);
    if (error != 0 || ferror(test_file) != 0) {
        perror("seeking test file");
        error = -1;
        goto out;
    }

    read = fread(signature, 1, sizeof(signature), test_file);
    if (read != 0 || ferror(test_file) != 0) {
        perror("reading test file");
        error = -1;
        goto out;
    }
    printf("signatized %s\n", signature);

    error = crypto_sign_verify_detached(signature, hash, sizeof(hash),
            public_key);
    if (error != 0) {
        fprintf(stderr, "Failed to verify file %s %d\n", file_name, error);
    }

out:
    fclose(public_key_file);
    fclose(test_file);
    return error;
}

int sign_file(char *secret_key_name, char *file_name, char *out_file_name) {
    int error = 0;
    crypto_hash_sha512_state state;
    unsigned char secret_key[crypto_sign_SECRETKEYBYTES];
    unsigned char hash[crypto_hash_sha512_BYTES];
    unsigned long long signature_final_size;
    unsigned char signature[crypto_sign_BYTES];
    unsigned char file_contents[512];
    size_t read;
    FILE *secret_key_file = NULL;
    FILE *target_file = NULL;
    FILE *out_file = NULL;

    secret_key_file = fopen(secret_key_name, "r");
    if (secret_key_file == NULL) {
        perror("opening secret key file");
        error = -1;
        goto out;
    }

    read = fread(secret_key, 1, sizeof(secret_key), secret_key_file);
    if (read != sizeof(secret_key)) {
        fprintf(stderr, "Invalid secret key file\n");
        error = -1;
        goto out;
    } else if (ferror(secret_key_file) != 0) {
        perror("reading secret key file");
        error = -1;
        goto out;
    }

    target_file = fopen(file_name, "r");
    if (target_file == NULL) {
        perror("opening target file");
        error = -1;
        goto out;
    }

    out_file = fopen(out_file_name, "w");
    if (out_file == NULL) {
        perror("opening output file");
        error = -1;
        goto out;
    }

    crypto_hash_sha512_init(&state);

    while (read != 0) {
        read = fread(file_contents, 1, sizeof(file_contents), target_file);
        if (ferror(target_file) != 0) {
            perror("reading target file");
            error = -1;
            goto out;
        }

        fwrite(file_contents, 1, read, out_file);
        if (ferror(out_file) != 0) {
            perror("writing out file");
            error = -1;
            goto out;
        }

        error = crypto_hash_sha512_update(&state, file_contents, read);
        if (error != 0) {
            fprintf(stderr, "Updating hash\n");
            error = -1;
            goto out;
        }
    }

    error = crypto_hash_sha512_final(&state, hash);
    if (error != 0) {
        fprintf(stderr, "Finalizing hash\n");
        error = -1;
        goto out;
    }

    error = crypto_sign_detached(signature, &signature_final_size, hash,
                                 sizeof(hash), secret_key);
    if (error != 0) {
        fprintf(stderr, "Signature\n");
        error = -1;
        goto out;
    }

    fwrite(SIGNATURE_MAGIC_STRING, sizeof(SIGNATURE_MAGIC_STRING), 1,
           out_file);
    if (ferror(out_file) != 0) {
        perror("writing magic string to out file");
        error = -1;
        goto out;
    }

    fwrite(&signature, sizeof(signature), 1, out_file);
    if (ferror(out_file) != 0) {
        perror("writing out file");
        error = -1;
        goto out;
    }
    printf("signatized %s\n", signature);

out:
    fclose(target_file);
    fclose(secret_key_file);
    fclose(out_file);
    return error;
}

int generate_key(char *secret_key_name, char *public_key_name) {
    int error = 0;
    unsigned char public_key[crypto_sign_PUBLICKEYBYTES];
    unsigned char secret_key[crypto_sign_SECRETKEYBYTES];
    FILE *public_key_file = NULL;
    FILE *secret_key_file = NULL;

    error = crypto_sign_keypair(public_key, secret_key);
    if (error != 0) {
        fprintf(stderr, "Could not generate key pair\n");
        error = -1;
        goto out;
    }

    public_key_file = fopen(public_key_name, "w");
    if (public_key_file == NULL) {
        perror("public key file");
        error = -1;
        goto out;
    }

    secret_key_file = fopen(secret_key_name, "w");
    if (public_key_file == NULL) {
        perror("secret key file");
        error = -1;
        goto out;
    }

    fwrite(public_key, sizeof(public_key), 1, public_key_file);
    if (ferror(public_key_file) != 0) {
        perror("writing to public key file");
        error = -1;
        goto out;
    }

    fwrite(secret_key, sizeof(secret_key), 1, secret_key_file);
    if (ferror(secret_key_file) != 0) {
        perror("writing to secret key file");
        error = -1;
        goto out;
    }

out:
    fclose(public_key_file);
    fclose(secret_key_file);
    return error;
}


int main(int argc, char *argv[]) {
    if (argc == 4 && strcmp(argv[1], "generate") == 0) {
        return generate_key(argv[2], argv[3]);
    } else if (argc == 5 && strcmp(argv[1], "sign") == 0) {
        return sign_file(argv[2], argv[3], argv[4]);
    } else if (argc == 4 && strcmp(argv[1], "verify") == 0) {
        return verify_file(argv[2], argv[3]);
        return -1;
    } else {
        return usage(argc, argv);
    }
}
