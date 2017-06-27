#pragma once

#include <truth/types.h>

enum status checked symbol_init(void);

void symbol_fini(void);

enum status checked symbol_remove(const char *symbol);

void *symbol_get(const char *symbol);

enum status checked symbol_put(const char *symbol, void *location);
