#include <truth/panic.h>
#include <truth/lock.h>

void lock_clear(struct lock *lock) {
    lock->writer = ATOMIC_BOOL_LOCK_FREE;
    lock->readers = ATOMIC_INT_LOCK_FREE;
}

void lock_acquire_reader(struct lock *lock) {
    atomic_bool expected = false;
    while (!atomic_compare_exchange_weak_explicit(&lock->writer, &expected, true, memory_order_acq_rel, memory_order_acq_rel));
    atomic_fetch_add_explicit(&lock->readers, 1, memory_order_acq_rel);
    atomic_store_explicit(&lock->writer, false, memory_order_acq_rel);
}

void lock_release_reader(struct lock *lock) {
    atomic_fetch_sub_explicit(&lock->readers, 1, memory_order_acq_rel);
}

void lock_acquire_writer(struct lock *lock) {
    atomic_bool expected_writer = false;
    while (!atomic_compare_exchange_weak_explicit(&lock->writer, &expected_writer, true, memory_order_acq_rel, memory_order_acq_rel)) {
        while (atomic_load_explicit(&lock->readers, memory_order_acq_rel) == 0);
    }
}

void lock_release_writer(struct lock *lock) {
    assert(atomic_load_explicit(&lock->writer, memory_order_acq_rel) == true);
    assert(atomic_load_explicit(&lock->readers, memory_order_acq_rel) == 0);
    atomic_store_explicit(&lock->writer, false, memory_order_acq_rel);
}

bool lock_is_writer_acquired(struct lock *lock) {
    return atomic_load_explicit(&lock->writer, memory_order_acq_rel);
}

bool lock_is_reader_acquired(struct lock *lock) {
    return atomic_load_explicit(&lock->readers, memory_order_acq_rel) > 0;
}
