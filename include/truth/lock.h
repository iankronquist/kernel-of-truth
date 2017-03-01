#pragma once

#include <truth/types.h>

struct lock {
    atomic_uint readers;
    atomic_bool writer;
};

void lock_acquire_reader(struct lock *lock);
void lock_release_reader(struct lock *lock);

void lock_acquire_writer(struct lock *lock);
void lock_release_writer(struct lock *lock);

#define Clear_Lock { .readers = 0, .writer = false }
