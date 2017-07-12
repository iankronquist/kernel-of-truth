#pragma once

#include <truth/types.h>

void mt19937_64_seed(uint64_t seed);

uint64_t mt19937_64_get_random_number(void);
