#include <stdarg.h>

#include <truth/file.h>
#include <truth/log.h>
#include <truth/types.h>

extern struct file *Log_File;

enum status init_log(const char *name) {
    return Log_File->init(name);
}

void log(const char *message) {
    fprintf(Log_File, "%s\n", message);
}

void logf(const char *message, ...) {
    va_list args;
    va_start(args, message);
    vfprintf(Log_File, message, args);
    va_end(args);
}
