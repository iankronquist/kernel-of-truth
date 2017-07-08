#include <truth/entropy.h>
#include <truth/jitter.h>
#include <truth/lock.h>

#define Entropy_Pool_Size 256

// FIXME: Use something more efficient than a spinlock!
static struct lock entropy_lock = Lock_Clear;
int Entropy_Pool_Index = 0;
uint8_t Entropy_Pool[Entropy_Pool_Size];


void entropy_pool_seed(uint8_t byte) {
    lock_acquire_writer(&entropy_lock);
    Entropy_Pool_Index++;
    Entropy_Pool[Entropy_Pool_Index] = byte;
    lock_release_writer(&entropy_lock);
}


uint8_t entropy_pool_consume(void) {

    lock_acquire_writer(&entropy_lock);
    if (Entropy_Pool_Index < 0) {
        lock_release_writer(&entropy_lock);
        uint64_t memory_jitter_entropy = memory_jitter_calculate();
        for (size_t i = 0; i < sizeof(memory_jitter_entropy); ++i) {
            entropy_pool_seed(memory_jitter_entropy & 0xff);
            memory_jitter_entropy >>= 8;
        }
        lock_acquire_writer(&entropy_lock);
    }
    uint8_t value = Entropy_Pool[Entropy_Pool_Index];
    Entropy_Pool_Index--;
    lock_release_writer(&entropy_lock);
    return value;
}
