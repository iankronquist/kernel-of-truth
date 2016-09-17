#include <stdarg.h>

#include <truth/file.h>
#include <truth/log.h>
#include <truth/types.h>

extern struct file *Log_File;

enum status init_log(string name) {
    return Log_File->init(name);
}

void log(string message) {
    fprintf(Log_File, "%s\n", message);
}

void logf(string message, ...) {
    va_list args;
    va_start(args, message);
    fprintf(Log_File, message, args);
    va_end(args);
}
