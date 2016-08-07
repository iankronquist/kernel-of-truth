#pragma once

#include <truth/types.h>

// Initialize any CPU features or tables needed for kernel operation.
// On x86 this includes the GDT.
void init_cpu(void);

// Initialize interrupt tables and exception handlers.
void init_interrupts(void);

// Initialize kernel logging.
void init_logging(void);

// Initialize any structures needed for tracking physical and virtual memory.
// This includes the kernel heap.
// boot_tables are a pointer to any processor specific memory tables built by
// the bootloader and inherited by the kernel. On multiboot platforms this is
// the multiboot tables.
void init_memory(void *boot_tables);

// Initialize multi-processing.
void init_multitasking(void);
