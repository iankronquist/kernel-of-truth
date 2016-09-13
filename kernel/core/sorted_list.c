#include <truth/sorted_list.h>
//#include <truth/heap.h>

/*
struct sorted_list *init_sorted_list(sorted_list_compare_f compare) {
    struct sorted_list *list = kmalloc(sizeof(struct sorted_list));
    list->compare = compare;
    list->entries = NULL;
    return list;
}
*/

bool sorted_list_contains(struct sorted_list *list,
        struct sorted_list_entry *entry) {
    struct sorted_list_entry *cur = list->entries;
    while (cur != NULL) {
        if (list->compare(entry, cur) == Order_Equal) {
            return true;
        }
        cur = cur->next;
    }
    return false;
}

void sorted_list_insert(struct sorted_list *list,
        struct sorted_list_entry *entry) {
    struct sorted_list_entry *cur = list->entries;
    if (cur == NULL) {
        list->entries = entry;
    }
    while (cur != NULL) {
        if (list->compare(entry, cur) == Order_Less) {
            cur = cur->next;
        } else {
            entry->next = cur->next;
            cur->next = entry;
        }
        cur = cur->next;
    }
    entry->next = NULL;
}

void sorted_list_remove(struct sorted_list *list,
        struct sorted_list_entry *entry) {
    struct sorted_list_entry *cur = list->entries;
    struct sorted_list_entry *prev = NULL;
    while (cur != NULL) {
        if (list->compare(entry, cur) == Order_Equal) {
            if (prev != NULL) {
                prev->next = cur->next;
            } else {
                list->entries = cur->next;
            }
            cur->next = NULL;
            return;
        }
        cur = cur->next;
    }
}
