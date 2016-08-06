#ifndef KLOG_H
#define KLOG_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <drivers/serial_port.h>

/* Initialize the kernel logging system.
 * Currently logs to the COM1 serial port.
 */
void initialize_klog();
/* Log a single string message. */
void klog(char *message);
/* Log a printf-style formatted message */
void klogf(char* string, ...);

#endif
