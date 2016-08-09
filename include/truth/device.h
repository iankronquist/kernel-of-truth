#pragma once

#include <truth/types.h>

struct device;

enum device_type {
    char_device,
    block_device,
};

struct device_char {
    status_t checked (*init)(const struct device *, int device_number,
            void *args);
    void (*fini)(const struct device *);
    ssize_t (*write)(const struct device *, char *buf, size_t size);
    int (*write_char)(const struct device *, char c);
    ssize_t (*write_string)(const struct device *, char *str);
    ssize_t (*read)(const struct device *, char *buf, size_t size);
    int (*read_char)(const struct device *);
};

struct device {
    char *name;
    int number;
    enum device_type type;
    union {
        struct device_char character;
    };
};

status_t checked register_device(const struct device *);
void unregister_device(const struct device *);

static inline struct device *get_device_from_char(const struct device *dc) {
    return (void*)dc - offsetof(const struct device, character);
}

// Character devices.
status_t checked device_char_init(const struct device *cdev, int device_number,
        void *args);
size_t device_char_puts(const struct device *cdev, char *string);
int device_char_putc(const struct device *cdev, char c);
ssize_t device_char_write(const struct device *cdev, char *buf, size_t bytes);
int device_char_getc(const struct device *cdev);
ssize_t device_char_read(const struct device *cdev, char *buf, size_t bytes);
