#ifndef KLOG_H
#define KLOG_H

#include <stddef.h>

#include <drivers/serial_port.h>

void initialize_klog();
void klog(char *message);

#endif
