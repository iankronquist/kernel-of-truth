#pragma once

#include <truth/types.h>

enum status init_log(const char *name);

void log(const char *message);

void logf(const char *message, ...) check_format(1, 2);
