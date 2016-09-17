#pragma once

#include <truth/types.h>

enum status init_log(string name);

void log(string message);

void logf(string message, ...);
