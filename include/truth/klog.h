#pragma once

/* Initialize the kernel logging system.
 * Currently logs to the COM1 serial port.
 */
void init_logging();
/* Log a single string message. */
void klog(char *message);
/* Log a printf-style formatted message */
void klogf(char* string, ...);
