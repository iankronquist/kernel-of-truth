#pragma once

#include <truth/types.h>
#include <truth/lock.h>

#define Object_Clear { .ref_count = 0, .free = NULL, .lock = Lock_Clear }

struct object;

typedef void (*object_free_f)(struct object *obj);

struct object {
    atomic_uint ref_count;
    object_free_f free;
    struct lock lock;
};

void object_retain(struct object *obj);
void object_release(struct object *obj);
void object_set_free(struct object *obj, object_free_f free_func);
