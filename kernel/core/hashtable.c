#include <truth/hashtable.h>
#include <truth/heap.h>
#include <truth/panic.h>
#include <truth/string.h>
#include <truth/types.h>

enum hashdata_state {
    Hashdata_Empty,
    Hashdata_Tombstone,
    Hashdata_Full,
};

struct hashdata {
    enum hashdata_state state;
    union hashtable_key key;
    void *value;
};

#define negative_golden_ratio 0x61C88647

// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.40.1573&rep=rep1&type=pdf
// Performance and Practice of String Hashing Functions
size_t hash_str(union hashtable_key str) {
    const char *cstr = str.ptr;
    size_t hash = negative_golden_ratio;
    for (size_t i = 0; cstr[i] != '\0'; ++i) {
        hash ^= cstr[i] + (hash << 5) + (hash >> 2);
    }
    return hash;
}

enum partial hash_str_comp(union hashtable_key key_a,
                           union hashtable_key key_b) {
    // Strings should not be longer than pages. That is nonsense.
    if (strncmp(key_a.ptr, key_b.ptr, Page_Small) != 0) {
        return Partial_Not_Equal;
    } else {
        return Partial_Equal;
    }
}

struct hashtable *hashtable_init(size_t size, hash_f hf, partial_comp_f hc) {
    struct hashtable *table = kmalloc(sizeof(struct hashtable));
    if (table == NULL) {
        return NULL;
    }
    table->comp = hc;
    table->hash = hf;
    table->size = size;
    table->used = 0;
    table->data = kmalloc(size * sizeof(struct hashdata));
    if (table->data == NULL) {
        kfree(table);
        return NULL;
    }
    memset(table->data, 0, size * sizeof(struct hashdata));
    return table;
}

void hashtable_destroy(struct hashtable *ht) {
    kfree(ht->data);
    kfree(ht);
}

static struct hashdata *hashtable_seek(struct hashtable *ht,
                                       union hashtable_key key) {
    size_t hash = ht->hash(key) % ht->size;
    if (ht->data[hash].state == Hashdata_Full &&
        ht->comp(key, ht->data[hash].key) == Partial_Equal) {
        return &ht->data[hash];
    }
    // There was a collision, probe the hash table.
    for (size_t s = (hash + 1) % ht->size; s != hash; s = (s + 1) % ht->size) {
        // If the item is empty, it would have been inserted here. Since it's
        // not here it's not in the hash table.
        if (ht->data[s].state == Hashdata_Empty) {
            return NULL;
        } else if (ht->data[s].state == Hashdata_Tombstone) {
            continue;
        } else if (ht->comp(key, ht->data[s].key) == Partial_Equal) {
            return &ht->data[s];
        }
    }
    // The hash table is full. Should never be reached.
    assert(Not_Reached);
    return NULL;
}

static struct hashdata *hashtable_seek_empty(struct hashtable *ht,
                                             union hashtable_key key) {
    size_t hash = ht->hash(key) % ht->size;
    if (ht->data[hash].state == Hashdata_Empty) {
        return &ht->data[hash];
    }
    // There was a collision, probe the hash table.
    for (size_t s = (hash + 1) % ht->size; s != hash; s = (s + 1) % ht->size) {
        // If the item is empty, it would have been inserted here. Since it's
        // not here it's not in the hash table.
        if (ht->data[s].state == Hashdata_Empty) {
            ht->data[s].state = Hashdata_Full;
            return &ht->data[s];
        }
    }
    // The hash table is full. Should never be reached.
    assert(Not_Reached);
    return NULL;
}

// If the hash table is more than 75% full, resize it.
// 75% was pulled from thin air because the math was easy.

// If more than a quarter is free.
static inline bool should_grow(size_t size, size_t used) {
    return size - used < size / 4;
}

// If less than a quarter is used and there are more than 32 entries.
static inline bool should_shrink(size_t size, size_t used) {
    return used < size / 4 && size < 32;
}

// New size must be large enough to hold all of the elements in old table.
static enum status checked hashtable_rebalance(struct hashtable *ht,
                                               size_t new_size) {
    assert(new_size > ht->size);
    struct hashtable new;
    new.used = 0;
    new.size = new_size;
    new.hash = ht->hash;
    new.comp = ht->comp;
    new.data = kmalloc(new.size * sizeof(struct hashdata));
    if (new.data == NULL) {
        return Error_No_Memory;
    }
    memset(new.data, 0, new.size * sizeof(struct hashdata));
    for (size_t s = 0; s < ht->size; ++s) {
        if (ht->data[s].state != Hashdata_Empty && ht->data[s].state !=
                Hashdata_Tombstone) {
            bubble(hashtable_put(&new, ht->data[s].key, ht->data[s].value),
                   "Rebuilding hashtable");
        }
    }

    kfree(ht->data);

    ht->data = new.data;
    ht->used = new.used;
    ht->size = new.size;
    return Ok;
}

enum status checked hashtable_remove(struct hashtable *ht,
                                     union hashtable_key key) {
    struct hashdata *hd = hashtable_seek(ht, key);
    hd->state = Hashdata_Tombstone;
    ht->used -= 1;
    if (should_shrink(ht->size, ht->used)) {
        return hashtable_rebalance(ht, ht->size / 2);
    }
    return Ok;
}

void *hashtable_get(struct hashtable *ht, union hashtable_key key) {
    struct hashdata *hd = hashtable_seek(ht, key);
    if (hd != NULL) {
        return hd->value;
    } else {
        return NULL;
    }
}

enum status checked hashtable_put(struct hashtable *ht,
                                  union hashtable_key key, void *value) {
    struct hashdata *hd = hashtable_seek_empty(ht, key);
    hd->key = key;
    hd->value = value;
    hd->state = Hashdata_Full;
    ht->used += 1;
    if (should_grow(ht->size, ht->used)) {
        return hashtable_rebalance(ht, ht->size * 2);
    }
    return Ok;
}
