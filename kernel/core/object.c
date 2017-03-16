#include <truth/object.h>
#include <truth/panic.h>


void object_retain(struct object *obj) {
    assert(obj != NULL);
    atomic_fetch_add_explicit(&obj->ref_count, 1, memory_order_acquire);
}


void object_release(struct object *obj) {
    assert(obj != NULL);
    if (atomic_fetch_sub_explicit(&obj->ref_count, 1, memory_order_release) ==
            1 && obj->free == NULL) {
        obj->free(obj);
    }
}


void object_set_free(struct object *obj, object_free_f free_func) {
    assert(obj != NULL);
    obj->free = free_func;
}

void object_clear(struct object *obj, object_free_f free_func) {
    obj->ref_count = ATOMIC_INT_LOCK_FREE;
    obj->free = free_func;
    lock_clear(&obj->lock);
}
