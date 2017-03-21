#pragma once

#include <stdarg.h>

#include <truth/types.h>
#include <truth/object.h>

enum file_type {
    File_Normal,
    File_Pipe,
    File_Directory,
    File_Device,
};

typedef enum status checked (init_f)(const char *name);
typedef enum status checked (open_f)(const char *name);
typedef enum status checked (read_f)(uint8_t *out, size_t size);
typedef enum status checked (write_f)(const uint8_t *in, size_t size);
typedef enum status checked (close_f)(const char *name);
typedef void (fini_f)(void);


struct file {
    enum file_type type;
    const char *name;
    init_f *init;
    fini_f *fini;
    read_f *read;
    write_f *write;
    struct file *parent;
    size_t child_capacity;
    size_t child_count;
    struct file **children;
    enum permissions permissions;
    struct object obj;
};

enum status file_system_init(void);
void file_system_fini(void);
enum status file_attach_path(const char *parent_path, struct file *child);
enum status file_attach(struct file *parent, struct file *child);
enum status file_detach(struct file *parent, struct file *child);
struct file *file_from_path(const char *path);
struct file *file_find_child_named(struct file *file, const char *name,
                                   size_t n);
void file_fini(struct file *file);

extern struct file Dev;
