#include <truth/cpu.h>
#include <truth/log.h>

static bool Panic = false;

void stack_trace(void) {
    uintptr_t **base_pointer = (uintptr_t **)get_base_pointer();
    while (base_pointer != 0) {
        uintptr_t **next_base_pointer = (uintptr_t **)base_pointer[0];
        uintptr_t *instruction_pointer = base_pointer[1];
        uintptr_t **arguments = &base_pointer[2];
        log("---------------------------------");
        logf("\tBase Pointer: %p\n", base_pointer);
        logf("\tInstruction Pointer: %p\n", instruction_pointer);
        logf("\tArguments: %p %p %p\n", arguments[0], arguments[1],
             arguments[2]);
        base_pointer = next_base_pointer;
    }
}


void panic(void) {
    disable_interrupts();
    if (Panic == false) {
        log("Kernel panic!");
        Panic = true;
        stack_trace();
    } else {
        log("Double panic!");
    }
    halt();
}
