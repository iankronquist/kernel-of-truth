#pragma once

#include <truth/file.h>
#include <truth/types.h>

extern struct file *Log_File;

enum log_level {
    Log_Error,
    Log_Warning,
    Log_Debug,
};

#define set_log_level(x) static enum log_level _log_level = x

#define init_log(name) Log_File->init(name)

#define log(level, message) \
    if (level <= _log_level) { \
        fprintf(Log_File, "%s\n", message); \
    }

#define logf(level, message, ...) \
    if (level <= _log_level) { \
        vfprintf(Log_File, message, ##__VA_ARGS__); \
    }
