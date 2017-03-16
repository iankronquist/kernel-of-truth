#pragma once

#include <truth/file.h>

struct device {
    struct file file;
};

extern struct device *log_device;

enum status checked device_init(struct device *device, char *name);
enum status checked read_device(struct device *device, char *in);
enum status checked write_device(struct device *device, char *out);
void device_fini(struct device *device);
