#include <truth/types.h>
#include <truth/lock.h>
#include <truth/panic.h>

#define W 64
#define N 312
#define M 156
#define R 31
#define A 0xb5026f5aa96619e9ull
#define U 29
#define D 0x5555555555555555ull
#define S 17
#define B 0x71d67fffeda60000ull
#define T 37
#define C 0xfff7eee000000000ull
#define L 43
#define F 6364136223846793005ull

#define Lower_Mask 0x7fffffffull
#define Upper_Mask (~Lower_Mask)

static uint64_t MT19937_64_Index = N + 1;
static uint64_t MT19937_64_State[N];

struct lock MT19937_64_Lock = Lock_Clear;

void mt19937_64_seed(uint64_t seed) {
    lock_acquire_writer(&MT19937_64_Lock);
    MT19937_64_State[0] = seed;
    for (size_t i = 1; i < N-1; ++i) {
        MT19937_64_State[i] = (F * (MT19937_64_State[i-1] ^ (MT19937_64_State[i-1] >> (W - 2))) + i);
    }
    MT19937_64_Index = 0;
    lock_release_writer(&MT19937_64_Lock);
}

static void mt19937_64_twist(void) {
    lock_acquire_writer(&MT19937_64_Lock);
    for (size_t i = 0; i < N-1; ++i) {
        uint64_t x = (MT19937_64_State[i] & Upper_Mask) + (MT19937_64_State[(i + 1) % N] & Lower_Mask);
        uint64_t xA = x >> 1;
        if ((x & 1) == 1) {
            xA ^= A;
        }
        MT19937_64_State[i] = MT19937_64_State[(i + M) % N] ^ xA;
    }
    MT19937_64_Index = 0;
    lock_release_writer(&MT19937_64_Lock);
}

uint64_t mt19937_64_get_random_number(void) {
    lock_acquire_writer(&MT19937_64_Lock);
    assert(MT19937_64_Index <= N);
    if (MT19937_64_Index == N) {
        mt19937_64_twist();
    }

    uint64_t y = MT19937_64_State[MT19937_64_Index];
    y ^= (y >> U) & D;
    y ^= (y << S) & B;
    y ^= (y << T) & C;
    y ^= y >> 1;
    MT19937_64_Index++;

    lock_release_writer(&MT19937_64_Lock);
    return y;
}
