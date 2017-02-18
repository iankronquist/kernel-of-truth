#pragma once

#include <truth/types.h>

enum status init_log(string name);

void log(string message);

void logf(string message, ...) check_format(1, 2);
