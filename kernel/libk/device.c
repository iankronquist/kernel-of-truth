#include <truth/device.h>
#include <truth/kassert.h>
#include <truth/kmem.h>
#include <truth/types.h>

struct device_element {
    const struct device *dev;
    struct device_element *next;
};

struct device_tree {
    struct device_element *list;
} Device_tree;

void unregister_device(const struct device *dev) {
    struct device_element *prev = NULL;
    struct device_element *de = Device_tree.list;
    while (de != NULL) {
        if (de->dev == dev) {
            if (prev != NULL) {
                prev->next = de->next;
            } else {
                Device_tree.list = NULL;
            }
        }
        prev = de;
        de = de->next;
    }
}

status_t checked register_device(const struct device *dev) {
    struct device_element *de = kmalloc(sizeof(struct device_element));
    de->dev = dev;
    de->next = Device_tree.list;
    Device_tree.list = NULL;
    return Ok;
}

status_t checked device_char_init(const struct device  *cdev, int device_number,
        void *args) {
    kassert(cdev != NULL);
    status_t stat = cdev->character.init(cdev, device_number, args);
    if (stat != Ok) {
        return stat;
    }
    return register_device(cdev);
}

size_t device_char_puts(const struct device *cdev, char *string) {
    size_t i;
    for (i = 0; string[i] != '\0'; ++i) {
        cdev->character.write_char(cdev, string[i]);
    }
    return i;
}

int device_char_putc(const struct device *cdev, char c) {
    return cdev->character.write_char(cdev, c);
}

ssize_t device_char_write(const struct device *cdev, char *buf, size_t bytes) {
    return cdev->character.write(cdev, buf, bytes);
}

int device_char_getc(const struct device *cdev) {
    return cdev->character.read_char(cdev);
}

ssize_t device_char_read(const struct device *cdev, char *buf, size_t bytes) {
    return cdev->character.read(cdev, buf, bytes);
}
