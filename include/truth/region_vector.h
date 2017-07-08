#pragma once

#include <truth/types.h>

struct region_vector;

void region_vector_init(struct region_vector *vect);

void region_vector_fini(struct region_vector *vect);

enum status checked region_alloc(struct region_vector *vect, size_t size, void **out);
enum status checked region_alloc_random(struct region_vector *vect, size_t size, void **out);

void region_free(struct region_vector *vect, void *address, size_t size);

size_t region_find_size_and_free(struct region_vector *vect, void *address);

struct region_vector *region_vector_new(void *addr, size_t size);
void debug_region_vector(struct region_vector *cur);
