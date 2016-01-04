#include <libk/klog.h>

void initialize_klog() {
    initialize_serial_port(COM1);
    write_serial_string(COM1, "Logger initialized!\n");
}
void klog(char *message) {
    write_serial_string(COM1, message);
}
