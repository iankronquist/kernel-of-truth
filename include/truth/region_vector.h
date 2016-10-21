#pragma once

#include <truth/types.h>

union address {
    byte *bytes;
    void *virtual;
    phys_addr physical;
};

struct region_vector;

void init_region_vector(struct region_vector *vect);

enum status checked region_alloc(struct region_vector *vect, size_t size,
                                 union address *out);

void region_free(struct region_vector *vect, union address address,
                 size_t size);

size_t region_find_size_and_free(struct region_vector *vect,
                                 union address address);

void debug_region_vector(struct region_vector *cur);
