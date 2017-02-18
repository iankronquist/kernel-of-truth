#include <stdarg.h>

#include <truth/file.h>
#include <truth/log.h>
#include <truth/types.h>

extern struct file *Log_File;

enum status init_log(char *name) {
    return Log_File->init(name);
}

void log(char *message) {
    fprintf(Log_File, "%s\n", message);
}

void logf(char *message, ...) {
    va_list args;
    va_start(args, message);
    vfprintf(Log_File, message, args);
    va_end(args);
}
