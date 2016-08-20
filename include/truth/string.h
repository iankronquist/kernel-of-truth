#ifndef TLIBC_STRING_H
#define TLIBC_STRING_H
#include <stddef.h>

/* All of these functions should behave like their C standard library
 * counterparts.
 */

// Compare bytes of two regions of memory.
int memcmp(const void*, const void*, size_t);
// Copy memory from one region to another. Regions must not overlap.
void* memcpy(void*, const void*, size_t);
// Copy memory from one region to another. Regions may overlap.
void* memmove(void*, const void*, size_t);
// Fill a region of memory with a value.
void* memset(void*, int, size_t);
// Get the length of a NULL terminates C string.
size_t strlen(const char* str);

#endif
