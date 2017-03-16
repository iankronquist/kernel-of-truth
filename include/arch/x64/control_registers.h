#pragma once

#include <truth/types.h>

void *read_cr2();
void write_cr3(phys_addr value);
uint64_t read_cr3(void);
