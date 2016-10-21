#pragma once

#include <truth/types.h>

// Function used for hashing an object. Should return a good hash in the range
// [0, SIZE_T_MAX].
typedef size_t (*hash_f)(void *);

// Compare two objects when fetching from a hash table.
typedef enum partial (*partial_comp_f)(void *, void *);

// A structure for holding key value pairs for the hash table.
struct hashdata;

// A hash table. When removing from the hash table, the item is not freed.
// When inserting the item is not copied.
struct hashtable {
    size_t used;
    size_t size;
    hash_f hash;
    partial_comp_f comp;
    struct hashdata *data;
};

// Hash a string.
size_t hash_str(void *str);

// Compare two strings.
enum partial hash_str_comp(void *key_a, void *key_b);

// Create and allocate a new hash table.
struct hashtable *hashtable_init(size_t size, hash_f hf, partial_comp_f hc);

// Free a hash table.
void hashtable_destroy(struct hashtable *ht);


// Remove an item from the hash table.
// Does not free the item.
// May resize the hash table.
enum status checked hashtable_remove(struct hashtable *ht, void *key);

// Get an item associated with a key from the hash table.
// Return NULL if the item is absent.
void *hashtable_get(struct hashtable *ht, void *key);

// Put an item in the hash table.
// Does not copy the item.
// May resize the hash table.
enum status checked hashtable_put(struct hashtable *ht, void *key,
                                  void *value);
