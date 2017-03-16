#pragma once

#include <truth/types.h>

struct bitmap {
    size_t entry_count;
    size_t last_free;
    unsigned long map[];
};


void bitmap_set(struct bitmap *bitmap, size_t entry);
void bitmap_clear(struct bitmap *bitmap, size_t entry);
void bitmap_set_range(struct bitmap *bitmap, size_t start, size_t length);
void bitmap_static_init(struct bitmap *bitmap, size_t entry_count);
struct bitmap *bitmap_init(size_t entry_count);
