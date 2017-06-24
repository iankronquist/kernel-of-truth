#include <truth/hashtable.h>
#include <truth/log.h>

#define SYMBOL_FUDGE_FACTOR 128

extern struct symbol_embedded kernel_symbol_table_start;
extern struct symbol_embedded kernel_symbol_table_end;

struct symbol_embedded {
    void *location;
    char *name;
};

static struct hashtable *symbol_table;

enum status checked symbol_init(void) {
    enum status status;
    union hashtable_key key;
    log(Log_Debug, "here 0");
    struct symbol_embedded *symbols = &kernel_symbol_table_start;
    size_t symbol_count = (&kernel_symbol_table_end -
                           &kernel_symbol_table_start) /
                                sizeof(struct symbol_embedded);
    symbol_table = hashtable_init(symbol_count + SYMBOL_FUDGE_FACTOR, hash_str,
                                  hash_str_comp);
    if (symbol_table == NULL) {
        return Error_No_Memory;
    }

    for (size_t i = 0; i < symbol_count; ++i) {
        key.ptr = symbols[i].name;
        status = hashtable_put(symbol_table, key, symbols[i].location);
        if (status != Ok) {
            for (--i; i != SIZE_MAX; --i) {
                key.ptr = symbols[i].name;
                enum status unused(_) = hashtable_remove(symbol_table, key);
            }
            return status;
        }

    }

    return Ok;
}

void symbol_fini(void) {
    // FIXME: Ensure all symbols have been removed
    hashtable_destroy(symbol_table);
}

enum status checked symbol_remove(char *symbol) {
    union hashtable_key key;
    key.ptr = symbol;
    return hashtable_remove(symbol_table, key);
}

void *symbol_get(char *symbol) {
    union hashtable_key key;
    key.ptr = symbol;
    return hashtable_get(symbol_table, key);
}

enum status checked symbol_put(char *symbol, void *location) {
    union hashtable_key key;
    key.ptr = symbol;
    return hashtable_put(symbol_table, key, location);
}
