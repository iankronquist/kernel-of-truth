#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <truth/kabort.h>
#include <truth/kassert.h>
#include <truth/kmem.h>
#include <truth/hashtable.h>

#define TOMBSTONE ((void*)0x1)
#define EMPTY NULL

struct hashdata {
    void *key;
    void *value;
};
#define NEG_GOLDEN_RATIO 0x61C88647
// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.40.1573&rep=rep1&type=pdf
// Performance and Practice of String Hashing Functions
size_t hash_str(void *str) {
    char *cstr = str;
    size_t hash = NEG_GOLDEN_RATIO;
    for (size_t i = 0; cstr[i]; ++i) {
        hash ^= cstr[i] + (hash << 5) + (hash >> 2);
    }
    return hash;
}

bool hash_str_comp(void *key_a, void *key_b) {
    // Strings should not be longer than pages. That is nonsense.
    return !strncmp(key_a, key_b, 4096);
}

struct hashtable *hashtable_init(size_t size, hashfunc hf, hashcomp hc) {
    struct hashtable *table = kmalloc(sizeof(struct hashtable));
    if (table == NULL) {
        return table;
    }
    table->comp = hc;
    table->hash = hf;
    table->size = size;
    table->used = 0;
    table->data = kmalloc(size * sizeof(struct hashdata));
    memset(table->data, (int)EMPTY, size * sizeof(struct hashdata));
    return table;
}

void hashtable_destroy(struct hashtable *ht) {
    kfree(ht->data);
    kfree(ht);
}

static struct hashdata *hashtable_seek(struct hashtable *ht, void *key) {
    size_t hash = ht->hash(key) % ht->size;
    if (ht->data[hash].key > TOMBSTONE && ht->comp(key, ht->data[hash].key)) {
        return &ht->data[hash];
    }
    // There was a collision, probe the hash table.
    for (size_t s = (hash + 1) % ht->size; s != hash; s = (s + 1) % ht->size) {
        // If the item is empty, it would have been inserted here. Since it's
        // not here it's not in the hash table.
        if (ht->data[s].key == EMPTY) {
            return NULL;
        } else if (ht->data[s].key == TOMBSTONE) {
            continue;
        } else if (ht->comp(key, ht->data[s].key)) {
            return &ht->data[s];
        }
    }
    kassert(false);
    // The hash table is full. Should never be reached.
    return NULL;
}

static struct hashdata *hashtable_seek_empty(struct hashtable *ht, void *key) {
    size_t hash = ht->hash(key) % ht->size;
    if (ht->data[hash].key == NULL) {
        return &ht->data[hash];
    }
    // There was a collision, probe the hash table.
    for (size_t s = (hash + 1) % ht->size; s != hash; s = (s + 1) % ht->size) {
        // If the item is empty, it would have been inserted here. Since it's
        // not here it's not in the hash table.
        if (ht->data[s].key == EMPTY) {
            return &ht->data[s];
        }
    }
    kassert(false);
    // The hash table is full. Should never be reached.
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
static int hashtable_rebalance(struct hashtable *ht, size_t new_size) {
    kassert(new_size > ht->size);
    struct hashtable new;
    new.used = 0;
    new.size = new_size;
    new.hash = ht->hash;
    new.comp = ht->comp;
    new.data = kmalloc(new.size * sizeof(struct hashdata));
    if (new.data == NULL) {
        return -1;
    }
    memset(new.data, (int)EMPTY, new.size * sizeof(struct hashdata));
    for (size_t s = 0; s < ht->size; ++s) {
        if (ht->data[s].key != EMPTY && ht->data[s].key !=  TOMBSTONE) {
            hashtable_put(&new, ht->data[s].key, ht->data[s].value);
        }
    }

    kfree(ht->data);

    ht->data = new.data;
    ht->used = new.used;
    ht->size = new.size;
    return 0;
}

void hashtable_remove(struct hashtable *ht, void *key) {
    struct hashdata *hd = hashtable_seek(ht, key);
    hd->key = TOMBSTONE;
    ht->used -= 1;
    if (should_shrink(ht->size, ht->used)) {
        // Silently fail to rebalance the hash table.
        (void)hashtable_rebalance(ht, ht->size / 2);
    }
}

void *hashtable_get(struct hashtable *ht, void *key) {
    struct hashdata *hd = hashtable_seek(ht, key);
    if (hd != NULL) {
        return hd->value;
    } else {
        return NULL;
    }
}

int hashtable_put(struct hashtable *ht, void *key, void *value) {
    struct hashdata *hd = hashtable_seek_empty(ht, key);
    hd->key = key;
    hd->value = value;
    ht->used += 1;
    if (should_grow(ht->size, ht->used)) {
        return hashtable_rebalance(ht, ht->size * 2);
    }
    return 0;
}
