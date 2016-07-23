#ifndef TLIBC_STRING_H
#define TLIBC_STRING_H
#include <stddef.h>

int memcmp(const void*, const void*, size_t);
void* memcpy(void*, const void*, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
size_t strlen(const char* str);
size_t strnlen(const char *s, size_t maxlen);
char *strncpy(char * dst, const char * src, size_t len);
size_t strncmp(const char *s1, const char *s2, size_t n);
#endif
