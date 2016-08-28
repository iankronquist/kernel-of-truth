#include <truth/device.h>
#include <truth/types.h>

status_t checked init(const struct device *unused(device),
        int unused(device_number), void *unused(args)) {
    return Ok;
}

void fini(const struct device *unused(dev)) {
    return;
}

ssize_t write(const struct device *unused(dev), char *unused(buf),
        size_t unused(size)) {
    return 0;
}

int write_char(const struct device *unused(dev), char unused(c)) {
    return 0;
}

ssize_t read(const struct device *unused(dev), char *unused(buf), size_t unused(size)) {
    return 0;
}

int read_char(const struct device *unused(dev)) {
    return 0;
}

struct device null_char_device = {
    .number = 0,
    .name = "null",
    .type = char_device,
    .character = {
        .fini = fini,
        .init = init,
        .write = write,
        .write_char = write_char,
        .read = read,
        .read_char = read_char,
    },
};

struct device *terminal_char_device = &null_char_device;
