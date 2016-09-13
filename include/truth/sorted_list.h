#pragma once

#include <truth/types.h>

struct sorted_list_entry {
    struct sorted_list_entry *next;
};

typedef enum order (*sorted_list_compare_f)(struct sorted_list_entry *left,
        struct sorted_list_entry *right);

struct sorted_list {
    sorted_list_compare_f compare;
    struct sorted_list_entry *entries;
};

struct sorted_list *init_sorted_list(sorted_list_compare_f compare);
bool sorted_list_contains(struct sorted_list *list,
        struct sorted_list_entry *entry);
void sorted_list_insert(struct sorted_list *list,
        struct sorted_list_entry *entry);
void sorted_list_remove(struct sorted_list *list,
        struct sorted_list_entry *entry);
