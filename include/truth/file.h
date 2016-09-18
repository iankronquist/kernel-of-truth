#pragma once

#include <stdarg.h>

#include <truth/types.h>

typedef enum status checked (init_f)(string name);
typedef enum status checked (read_f)(byte *out, size_t size);
typedef enum status checked (write_f)(const byte *in, size_t size);
typedef void (fini_f)(void);

struct file {
    string name;
    init_f *init;
    fini_f *fini;
    read_f *read;
    write_f *write;
    struct file *parent;
    struct file **children;
    atomic_uint references;
    enum permissions permissions;
};

enum status vfprintf(struct file *file, const char *restrict format,
                     va_list ap);

enum status fprintf(struct file *file, const char *restrict format, ...);
