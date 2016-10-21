#pragma once

#include <truth/types.h>

union region_address {
    byte *bytes;
    void *virtual;
    phys_addr physical;
};

struct region {
    int tag,
    union region_address address;
    size_t size;
};

struct region_vector_size_sorted {
    size_t regions_used;
    struct region_vector_size_sorted *next;
    struct region regions[];
};

struct region_vector_address_sorted {
    size_t regions_used;
    struct region_vector_address_sorted *next;
    struct region regions[];
};

struct region_vector;

void init_region_vector_size_sorted(struct region_vector_size_sorted *vect);
void init_region_vector_address_sorted(struct region_vector_address_sorted *v);

enum status checked region_alloc_size_sorted(struct region_vector *vect,
                                             size_t size,
                                             union region_address *out);

void region_free_size_sorted(struct region_vector_size_sorted *vect,
                             union region_address address, size_t size);

enum status checked region_alloc_address_sorted(struct region_vector *vect,
                                             size_t size,
                                             union region_address *out);

void region_free_address_sorted(struct region_vector_address_sorted *vect,
                             union region_address address, size_t size);

void debug_region_vector(struct region_vector *cur);
