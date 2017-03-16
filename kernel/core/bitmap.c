#include <truth/bitmap.h>
#include <truth/heap.h>
#include <truth/panic.h>
#include <truth/string.h>
#include <truth/types.h>

#define bitmap_index(x) ((x) / sizeof(unsigned long))
#define bitmap_bit(x) (1 << (x) % sizeof(unsigned long))
#define bitmap_entry_count_to_bytes(x) ((x) / 8)


size_t log2(unsigned long item) {
    size_t i = 0;
    while (item >>= 1) {
        i++;
    }
    return i;
}


enum status bitmap_get_index(struct bitmap *bitmap, size_t *entry) {
    assert(bitmap != NULL);
    for (size_t i = bitmap_index(bitmap->last_free);
         i < bitmap_index(bitmap->entry_count);
         ++i) {
        if (bitmap->map[i] != ~0ul) {
            size_t entry_bit = log2(bitmap->map[i]);
            *entry = (i << sizeof(unsigned long)) + entry_bit;
            bitmap->map[i] |= 1 << entry_bit;
            return Ok;
        }
    }
    for (size_t i = 0; i < bitmap_index(bitmap->last_free); ++i) {
        if (bitmap->map[i] != ~0ul) {
            size_t entry_bit = log2(bitmap->map[i]);
            *entry = (i << sizeof(unsigned long)) + entry_bit;
            bitmap->map[i] |= 1 << entry_bit;
            return Ok;
        }
    }
    return Error_Full;
}


bool bitmap_is_set(struct bitmap *bitmap, size_t entry) {
    assert(bitmap != NULL);
    assert(bitmap->entry_count < entry);
    return bitmap->map[bitmap_index(entry)] & ~bitmap_bit(entry);
}


void bitmap_set(struct bitmap *bitmap, size_t entry) {
    assert(bitmap != NULL);
    assert(bitmap->entry_count < entry);
    bitmap->map[bitmap_index(entry)] |= bitmap_bit(entry);
}


void bitmap_clear(struct bitmap *bitmap, size_t entry) {
    assert(bitmap != NULL);
    assert(bitmap->entry_count < entry);
    bitmap->last_free = entry;
    bitmap->map[bitmap_index(entry)] &= ~bitmap_bit(entry);
}


void bitmap_set_range(struct bitmap *bitmap, size_t start, size_t length) {
    assert(bitmap != NULL);
    assert(bitmap->entry_count < start);
    assert(bitmap->entry_count < start + length);
    bitmap->map[bitmap_index(start)] = (bitmap_bit(start) - 1) |
                                       bitmap_bit(start);
    bitmap->map[bitmap_index(start + length)] = ~(bitmap_bit(start + length) -
                                                  1);
    for (size_t i = bitmap_index(start) + 1;
         i < bitmap_index(start + length) - 1;
         ++i) {
        bitmap->map[i] = ~0;
    }
}


void bitmap_static_init(struct bitmap *bitmap, size_t entry_count) {
    assert(bitmap != NULL);
    bitmap->entry_count = entry_count;
    bitmap->last_free = 0;
    memset(&bitmap->map, 0, bitmap_entry_count_to_bytes(entry_count));
}


struct bitmap *bitmap_init(size_t entry_count) {
    struct bitmap *bm = kmalloc(sizeof(size_t) +
                                bitmap_entry_count_to_bytes(entry_count));
    if (bm == NULL) {
        return NULL;
    }
    bm->last_free = 0;
    bm->entry_count = entry_count;
    memset(&bm->map, 0, bitmap_entry_count_to_bytes(entry_count));
    return bm;
}
