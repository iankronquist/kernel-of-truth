#ifndef KLOG_H
#define KLOG_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <drivers/serial_port.h>

void initialize_klog();
void klog(char *message);
void klogf(char* string, ...);

#endif
