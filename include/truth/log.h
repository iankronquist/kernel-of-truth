#pragma once

#include <truth/types.h>

enum status init_log(char *name);

void log(char *message);

void logf(char *message, ...) check_format(1, 2);
