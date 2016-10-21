#pragma once

#include <truth/types.h>

// A reader-writer spinlock.
struct lock {
    atomic_uint readers;
    atomic_uint writers;
};

void acquire_read_lock(struct lock *lock);
void acquire_write_lock(struct lock *lock);
void release_read_lock(struct lock *lock);
void release_write_lock(struct lock *lock);

#define clear_lock { 0, 0 }
