#pragma once

#include <truth/types.h>

// A device object holds information about a device driver. Devices have names,
// device numbers, types, and a series of functions which they will service
// when they are called.
struct device;

// Keeping with the Unix tradition, there are two types of devices, character
// devices and block devices.
enum device_type {
    char_device,
    block_device,
};

// A character device has several functions which define actions on the device.
// It has an init function for performing any initialization actions,
// and a fini function for performing any complementary cleanup.
// It also has read and write functions modeled after the read(2) and write(2)
// syscalls and read_char and write_char which have signatures similar to putc
// and getc.
// Additionally it has a write_string function for convenience which write a
// null terminated string byte by byte.
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

// The device structure represents evices attached to the kernel of truth.
// It has a name and number, as well as a tagged union indicating its device
// type.
struct device {
    char *name;
    int number;
    enum device_type type;
    // An anonymous union for the device's functions.
    union {
        struct device_char character;
    };
};

// Takes a device and registers it to the device tree.
status_t checked register_device(const struct device *);
// Takes a device and unregisters it from the device tree.
void unregister_device(const struct device *);

// Initialize a character device.
status_t checked device_char_init(const struct device *cdev, int device_number,
        void *args);
// Call puts on a character device.
size_t device_char_puts(const struct device *cdev, char *string);
// Call putc on a character device.
int device_char_putc(const struct device *cdev, char c);
// Call write on a character device.
ssize_t device_char_write(const struct device *cdev, char *buf, size_t bytes);
// Call getc on a character device.
int device_char_getc(const struct device *cdev);
// Call read on a character device.
ssize_t device_char_read(const struct device *cdev, char *buf, size_t bytes);
