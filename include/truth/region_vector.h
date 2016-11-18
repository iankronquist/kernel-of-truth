#pragma once

#include <truth/types.h>

union region_address {
    byte *bytes;
    void *virtual;
    phys_addr physical;
};

struct region_vector;

void init_region_vector(struct region_vector *vect);

enum status checked region_alloc(struct region_vector *vect,
                                 size_t size, union region_address *out,
                                 int tag);

enum status checked region_put_by_size(struct region_vector *vect,
                                       union region_address address,
                                       size_t size, int tag);

enum status checked region_get_by_size(struct region_vector *vect, size_t size,
                                       union region_address *address);

size_t region_get_by_address(struct region_vector *vect,
                             union region_address address, int tag);

enum status checked region_put_by_address(struct region_vector *vect,
                                          union region_address address,
                                          size_t size, int tag);

void debug_region_vector(struct region_vector *cur);
