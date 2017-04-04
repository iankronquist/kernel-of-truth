#include <truth/types.h>
#include <truth/panic.h>

uintptr_t __stack_chk_guard;

void __stack_chk_fail(void) {
    log(Log_Error, "Stack guard check failed");
    panic();
}
