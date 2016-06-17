#ifndef LOCK_H
#define LOCK_H
typedef int spinlock_t;

#define SPINLOCK_INIT 0

void acquire_spinlock(spinlock_t *s);

int release_spinlock(spinlock_t *s);
#endif
