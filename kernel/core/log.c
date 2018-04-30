#include <stdarg.h>

#include <truth/format.h>
#include <truth/log.h>
#include <truth/panic.h>
#include <truth/types.h>

extern struct file *Log_File;


static enum log_level Log_Global_Level = Log_None;


char *Log_Prefix[Log_Count] = {
    "Debug: ",
    "Info:  ",
    "Warn:  ",
    "Error: ",
    "",
};

const char *VMX_Level[2] = {
    "Guest: ",
    "Host:  ",
};


enum status log_init(enum log_level level, const char *name) {
    Log_Global_Level = level;
    return Log_File->init(name);
}


void log(enum log_level level, const char *message) {
    if (level >= Log_Global_Level) {
        fprintf(Log_File, "%s %s%s\n", Log_Prefix[level], VMX_Level[vmx_in_host()], message);
    }
}


void logf(enum log_level level, const char *message, ...) {
    if (level >= Log_Global_Level) {
        va_list args;
        va_start(args, message);
        fprintf(Log_File, "%s %s", Log_Prefix[level], VMX_Level[vmx_in_host()]);
        vfprintf(Log_File, message, args);
        va_end(args);
    }
}


void log_set_level(enum log_level level) {
    assert(level < Log_Count);
    Log_Global_Level = level;
}


void log_hexdump(enum log_level level, void *memory, size_t bytes) {
    if (level >= Log_Global_Level) {
        logf(level, "Hexdump starting at %p for %zu bytes\n", memory, bytes);
        // Print every byte.
        size_t i;
        unsigned char *m = memory;
        for (i = 0; i < bytes; ++i) {
            // Every 16 bytes, print the current index.
            if ((i % 16) == 0) {
                fprintf(Log_File, "%zx: ", i);
            }
            fprintf(Log_File, "%hhx", m[i]);
            // Every half word, print a space.
            if ((i % 2) == 1) {
                fprintf(Log_File, " ");
            }
            // After 16 bytes, print a newline.
            if ((i % 16) == 15) {
                fprintf(Log_File, "\n");
            }
        }
        // Don't put a double newline.
        if ((i % 16) != 15) {
            fprintf(Log_File, "\n");
        }
    }
}
