#ifndef KABORT_H
#define KABORT_H

// Kernel panic!
// Prints out a message and halts the CPU.
void kabort(void);

// Print a formatted message and kernel panic.
void kabort_message(char *format, ...);
#endif
