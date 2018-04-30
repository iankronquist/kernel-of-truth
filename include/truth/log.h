#pragma once

#include <truth/types.h>

enum log_level {
    Log_Debug,
    Log_Info,
    Log_Warning,
    Log_Error,
    Log_None,
    Log_Count,
};

enum status log_init(enum log_level level, const char *name);

void log_set_level(enum log_level level);

void log(enum log_level level, const char *message);

void logf(enum log_level level, const char *message, ...) check_format(2, 3);

void log_hexdump(enum log_level level, void *memory, size_t bytes);
