#ifndef GDT_H
#define GDT_H

#include <stdint.h>

/* Install the <Gdt> and set it up as described. */
void gdt_install(void);

#endif
