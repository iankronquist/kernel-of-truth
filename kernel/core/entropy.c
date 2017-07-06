#include <truth/entropy.h>

#define Entropy_Pool_Size 512

atomic_int pool_head = ATOMIC_INT_LOCK_FREE;
uint8_t entropy_pool[Entropy_Pool_Size];



void entropy_pool_seed(uint8_t byte) {
    int index = atomic_fetch_add_explicit(&pool_head, 1, memory_order_acquire);
    if (index >= Entropy_Pool_Size) {
        atomic_fetch_sub_explicit(&pool_head, 1, memory_order_acquire);
        return;
    }
    entropy_pool[index] = byte;
}

uint8_t entropy_pool_consume(void) {
    int index;
    while (true) {
        index = atomic_fetch_sub_explicit(&pool_head, 1, memory_order_acquire);
        if (index >= 0) {
            break;
        }
        atomic_fetch_add_explicit(&pool_head, 1, memory_order_acquire);
    }

    return entropy_pool[index];
}
