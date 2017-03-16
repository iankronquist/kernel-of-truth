#pragma once

#include <truth/types.h>

/* All of these functions should behave like their C standard library
 * counterparts. The normal variety of functions cannot touch user memory.
 * Functions prefixed with 'user_' can.
 */

// Compare bytes of two regions of kernel memory.
int memcmp(const void *, const void *, size_t);
// Copy kernel memory from one region to another. Regions must not overlap.
void *memcpy(void *, const void *, size_t);
// Copy kernel memory from one region to another. Regions may overlap.
void *memmove(void *, const void *, size_t);
// Fill a region of kernel memory with a value.
void *memset(void *, int, size_t);
// Get the length of a NULL terminated C string in kernel memory.
size_t strnlen(const char str, size_t size);
// Compare two strings in kernel memory.
enum order strncmp(const char *s1, const char *s2, size_t n);


// Compare bytes of two regions of memory.
int user_memcmp(const void *, const void *, size_t);
// Copy memory from one region to another. Regions must not overlap.
void *user_memcpy(void *, const void *, size_t);
// Copy memory from one region to another. Regions may overlap.
void *user_memmove(void *, const void *, size_t);
// Fill a region of memory with a value.
void *user_memset(void *, int, size_t);
// Get the length of a NULL terminates C string.
size_t user_strnlen(const char str, size_t size);
// Compare two strings.
enum order user_strncmp(const char *s1, const char *s2, size_t n);
