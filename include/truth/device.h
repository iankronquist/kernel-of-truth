#pragma once

#include <truth/file.h>

struct device {
    struct file file;
};

extern struct device *log_device;

enum status checked init_device(struct device *device, string name);
enum status checked read_device(struct device *device, string in);
enum status checked write_device(struct device *device, string out);
void fini_device(struct device *device);
