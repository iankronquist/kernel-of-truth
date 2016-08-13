#include <truth/hashtable.h>
#include <tests/tests.h>
char *abc[] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
    "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "A", "B",
    "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q",
    "R", "S", "T", "U", "V", "W", "X", "Y", "Z" };

#define THREE_QUARTERS(x) (x/2+x/4)
#define ONE_QUARTER(x) (x/4)

int main() {
    size_t old_size = 4;
    struct hashtable *ht = hashtable_init(old_size, hash_str, hash_str_comp);
    // Test insertion
    for (unsigned i = 0; i < sizeof(abc)/sizeof(char*); ++i) {
        hashtable_put(ht, abc[i], abc[i]);
        // If it's more than three quarters full, the size changes. Otherwise
        // it doesn't.
        if (ht->used > THREE_QUARTERS(old_size)) {
            EXPECT_GT(ht->size, old_size);
            old_size = ht->size;
        } else {
            EXPECT_EQ(ht->size, old_size);
        }
    }
    // Test presence
    for (unsigned i = 0; i < sizeof(abc)/sizeof(char*); ++i) {
        EXPECT_EQ(abc[i], hashtable_get(ht, abc[i]));
        EXPECT_EQ(ht->size, old_size);
    }
    // Test deletion
    for (unsigned i = 0; i < sizeof(abc)/sizeof(char*); ++i) {
        hashtable_remove(ht, abc[i]);
        // If it's more than three quarters full, the size changes. Otherwise
        // it doesn't.
        if (ht->used < ONE_QUARTER(old_size)) {
            EXPECT_LEQ(ht->size, old_size);
            old_size = ht->size;
        } else {
            EXPECT_EQ(ht->size, old_size);
        }
    }
    // Test absence
    for (unsigned i = 0; i < sizeof(abc)/sizeof(char*); ++i) {
        EXPECT_EQ(NULL, hashtable_get(ht, abc[i]));
        EXPECT_EQ(ht->size, old_size);
    }

    // Test that all of them are absent
    hashtable_destroy(ht);

    return RETURN_VALUE;
}
