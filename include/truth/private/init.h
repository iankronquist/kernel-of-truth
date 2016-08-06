#pragma once

#include <truth/types.h>

// Initialize any CPU features or tables needed for kernel operation.
// On x86 this includes the GDT.
// boot_tables are a pointer to any processor specific memory tables built by
// the bootloader and inherited by the kernel. On multiboot platforms this is
// the multiboot tables.
status_t init_cpu(void *boot_tables);

// Initialize interrupt tables and exception handlers.
status_t init_interrupts(void);

// Initialize any structures needed for tracking physical and virtual memory.
status_t init_memory(void);
