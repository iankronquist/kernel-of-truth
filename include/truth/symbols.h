#pragma once

#include <truth/types.h>

enum status checked symbol_init(void);

void symbol_fini(void);

enum status checked symbol_remove(char *symbol);

void *symbol_get(char *symbol);

enum status checked symbol_put(char *symbol, void *location);
