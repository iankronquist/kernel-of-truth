#include <stdio.h>
#include <stdarg.h>
#include <assert.h>


void kputs(char* string) {
    puts(string);
}

void kprintf(char* string, ...) {
    va_list a;
    va_start(a, string);
    vfprintf(stdout, string, a);
}

void kabort() {
    assert(0);
}
