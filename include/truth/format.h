#pragma once

#include <truth/file.h>


enum status vfprintf(struct file *file, const char *restrict format,
                     va_list ap);
enum status fprintf(struct file *file, const char *restrict format, ...);
