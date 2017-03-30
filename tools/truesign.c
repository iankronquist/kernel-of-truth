#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <truth/crypto.h>

#define DEBUG 0

#define SIGNATURE_MAGIC_STRING "Kernel of Truth truesign 1.0.0"


int usage(char *name) {
    fprintf(stderr,
            "Usage: %s <command> <args>\n"
            "Commands:\n"
            "\tgenerate privkey pubkey      \tGenerate an ed25519 key pair\n"
            "\tsign privkey in_file out_file\tSign file with public key\n"
            "\tverify pubkey file           \tVerify file with private key\n",
            name);
    return EXIT_FAILURE;
}


static int get_file_size(FILE *file, size_t *size) {
    long told;
    long original_position = ftell(file);
    if (original_position == -1) {
        return -1;
    }

    int error = fseek(file, 0, SEEK_END);
    if (error != 0) {
        return error;
    }

    told = ftell(file);
    if (told == -1) {
        return -1;
    }

    error = fseek(file, original_position, SEEK_SET);
    *size = told;

    return error;
}


#if DEBUG
void print_bytes(unsigned char *bytes, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        printf("%x%x", (bytes[i] >> 4) & 0x0f, bytes[i] & 0x0f);
    }
    printf("\n");
}
#endif


int verify_file(char *public_key_name, char *test_file_name) {
    int error = 0;
    char signature_magic[sizeof(SIGNATURE_MAGIC_STRING)];
    unsigned char signature[crypto_hash_sha512_BYTES];
    unsigned char public_key[crypto_sign_ed25519_PUBLICKEYBYTES];
    size_t read;
    size_t test_file_size;
    size_t signature_field_size = crypto_sign_ed25519_BYTES +
        sizeof(SIGNATURE_MAGIC_STRING);
    FILE *public_key_file = NULL;
    FILE *test_file = NULL;
    unsigned char *test_file_contents = NULL;

    public_key_file = fopen(public_key_name, "r");
    if (public_key_file == NULL) {
        perror("opening public key file");
        error = -1;
        goto out;
    }

    test_file = fopen(test_file_name, "r");
    if (test_file == NULL) {
        perror("opening test file");
        error = -1;
        goto out;
    }

    error = get_file_size(test_file, &test_file_size);
    if (error != 0) {
        perror("Reading test file size");
        goto out;
    } else if (test_file_size < crypto_sign_ed25519_PUBLICKEYBYTES) {
        fprintf(stderr, "File is too small to contain a signature\n");
        error = -1;
        goto out;
    }

    fread(public_key, 1, sizeof(public_key), public_key_file);
    if (ferror(public_key_file) != 0) {
        perror("Reading public key");
        error = -1;
        goto out;
    }

    test_file_contents = malloc(test_file_size);
    if (test_file_contents == NULL) {
        fprintf(stderr, "Couldn't allocate memory for test file contents\n");
        error = -1;
        goto out;
    }

    fread(test_file_contents, 1, test_file_size - signature_field_size,
          test_file);
    if (ferror(test_file) != 0) {
        perror("Reading data from test file");
        error = -1;
        goto out;
    }

    fread(signature_magic, 1, sizeof(signature_magic), test_file);
    if (ferror(test_file) != 0) {
        perror("Reading signature from test file");
        error = -1;
        goto out;
    }

    if (strncmp(SIGNATURE_MAGIC_STRING, signature_magic,
                sizeof(SIGNATURE_MAGIC_STRING)) != 0) {
        fprintf(stderr, "Bad signature magic\n");
        error = -1;
        goto out;
    }

    fread(signature, 1, sizeof(signature), test_file);
    if (ferror(test_file) != 0) {
        perror("Reading signature from test file");
        error = -1;
        goto out;
    }

    read = ftell(test_file);
    if (read != test_file_size) {
        fprintf(stderr, "Bug reading signature\n");
        error = -1;
        goto out;
    }

#if DEBUG
    printf("Public key:\n");
    print_bytes(public_key, sizeof(public_key));
    printf("Signature:\n");
    print_bytes(signature, sizeof(signature));
#endif

    error = crypto_sign_ed25519_verify_detached(signature, test_file_contents,
                                                test_file_size -
                                                    sizeof(signature) -
                                                    sizeof(signature_magic),
                                                 public_key);
    if (error != 0) {
        fprintf(stderr, "Failed to verify file %s\n", test_file_name);
        goto out;
    } else {
        printf("Signature successfully verified\n");
    }

out:
    free(test_file_contents);
    fclose(public_key_file);
    fclose(test_file);
    return error;
}


int sign_file(char *secret_key_name, char *file_name, char *out_file_name) {
    int error = 0;
    unsigned char secret_key[crypto_sign_ed25519_SECRETKEYBYTES];
    unsigned long long signature_final_size;
    unsigned char signature[crypto_sign_ed25519_BYTES];
    size_t file_size;
    size_t read;
    FILE *secret_key_file = NULL;
    FILE *target_file = NULL;
    FILE *out_file = NULL;
    unsigned char *file_contents = NULL;

    secret_key_file = fopen(secret_key_name, "r");
    if (secret_key_file == NULL) {
        perror("opening secret key file");
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

    error = get_file_size(target_file, &file_size);
    if (error != 0) {
        perror("Getting file size");
        goto out;
    }

    file_contents = malloc(file_size);
    if (file_contents == NULL) {
        fprintf(stderr, "Could not allocate memory\n");
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

    read = fread(file_contents, 1, file_size, target_file);
    if (ferror(target_file) != 0) {
        perror("Reading target file");
        error = -1;
        goto out;
    }

    fwrite(file_contents, 1, read, out_file);
    if (ferror(out_file) != 0) {
        perror("Writing out file");
        error = -1;
        goto out;
    }

    error = crypto_sign_ed25519_detached(signature, &signature_final_size,
                                         file_contents, file_size, secret_key);
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

#if DEBUG
    printf("Secret key:\n");
    print_bytes(secret_key, sizeof(secret_key));
    printf("Signature:\n");
    print_bytes(signature, sizeof(signature));
#endif

out:
    fclose(target_file);
    fclose(secret_key_file);
    fclose(out_file);
    free(file_contents);
    return error;
}


int generate_key(char *secret_key_name, char *public_key_name) {
    int error = 0;
    unsigned char public_key[crypto_sign_ed25519_PUBLICKEYBYTES];
    unsigned char secret_key[crypto_sign_ed25519_SECRETKEYBYTES];
    FILE *public_key_file = NULL;
    FILE *secret_key_file = NULL;

    error = crypto_sign_ed25519_keypair(public_key, secret_key);
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

#if DEBUG
    printf("Public key:\n");
    print_bytes(public_key, sizeof(public_key));
    printf("Secret key:\n");
    print_bytes(secret_key, sizeof(secret_key));
#endif


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
        return usage(argv[0]);
    }
}
