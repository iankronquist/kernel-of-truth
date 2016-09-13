#pragma once

#include <truth/types.h>

typedef enum status checked (*init_f(string name));
typedef enum status checked (*read_f(string out));
typedef enum status checked (*write_f(string in));
typedef void (*fini_f(void));

struct file {
    string name;
    init_f init;
    fini_f fini;
    read_f read;
    write_f write;
    struct file *parent;
    struct file **children;
    atomic_t references;
};
