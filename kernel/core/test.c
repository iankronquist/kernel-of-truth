#include <arch/x64/port.h>
#include <truth/panic.h>

#define TEST_RESULT_PORT_NUMBER 0xf4

void test_shutdown_status(enum status status) {
    logf(Log_Debug, "Test shutting down with status %s (%d)\n", status_message(status), status);
    write_port(status, TEST_RESULT_PORT_NUMBER);
	//__asm__("movb $0x0, %al; outb %al, $0xf4");
	__asm__("movb $0xf4, %al; outb %al, $0x0");
	__asm__("movb $0x0, %al; outb %al, $0xf4");
	halt();
    //write_port(2, TEST_RESULT_PORT_NUMBER);
    assert(Not_Reached);
}
