#include <truth/cpu.h>
#include <truth/types.h>
#include <truth/log.h>

set_log_level(Log_Debug);

void kernel_main(void) {
    init_interrupts();
    enum status unused(status) = init_log("log");
    log(Log_Debug, "Hello kernel!");
    while(true);
}
