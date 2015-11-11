#include <libk/kassert.h>

void kassert(int value)
{
    if (value) {
        return;
    }
    kputs("Assertion failed!");
    kabort();
}
