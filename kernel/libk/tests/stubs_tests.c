#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include <truth/lock.h>

// I should really include a more robust implementation of the spinlock
void acquire_spinlock(spinlock_t *s) {
    *s = 1;
}

int release_spinlock(spinlock_t *s) {
    int orig = *s;
    *s = 0;
    return orig;
}

void kputs(char* string) {
    puts(string);
}

void kprintf(char* string, ...) {
    va_list a;
    va_start(a, string);
    vfprintf(stdout, string, a);
}

void klogf(char* string, ...) {
    va_list a;
    va_start(a, string);
    vfprintf(stdout, string, a);
}

void kabort() {
    assert(0);
}
