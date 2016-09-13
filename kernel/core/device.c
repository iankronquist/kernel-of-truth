#include <truth/device.h>

enum status checked init_device(struct device *device, string name) {
    return init_file(device->file, name);
}

enum status checked read_device(struct device *device, string in) {
    return read_file(device->file, in);
}

enum status checked write_device(struct device *device, string out) {
    return write_file(device->file, out);
}

void fini_device(struct device *device) {
    fini_file(device);
}
