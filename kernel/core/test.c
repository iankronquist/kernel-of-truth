#include <arch/x64/port.h>
#include <truth/panic.h>

#define Test_Result_Port_Number 0xf4

void test_shutdown_status(enum status status) {
    logf(Log_Debug, "Test shutting down with status %s (%d)\n", status_message(status), status);
    write_port(status, Test_Result_Port_Number);
}
