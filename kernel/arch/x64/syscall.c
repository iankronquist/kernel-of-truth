#include <truth/syscall.h>
#include <truth/lock.h>
#include <truth/log.h>
#include <arch/x64/msr.h>


struct syscall_info {
    syscall_f function;
};

#define Flags_Clear_Interrupts (1 << 9)

extern void _syscall_entrypoint(void);


struct syscall_info Syscall_Table[Syscall_Max] = {{NULL, 0}};
struct lock Syscall_Table_Lock = Lock_Clear;


uint64_t syscall_entrypoint(uint64_t rsi, uint64_t rdi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t syscall_number) {
    uint64_t ret;
    // FIXME: kill thread?
    if (syscall_number > Syscall_Max) {
        log(Log_Error, "Bad syscall");
        return -1;
    }
    lock_acquire_reader(&Syscall_Table_Lock);

    ret = Syscall_Table[syscall_number].function(rsi, rdi, rdx, rcx, r8);

    lock_release_reader(&Syscall_Table_Lock);

    return ret;
}


enum status syscall_init(void) {
    wrmsr(IA32_FMASK_MSR, ~Flags_Clear_Interrupts);
    wrmsr(IA32_LSTAR_MSR, _syscall_entrypoint);
    return Ok;
}


void syscall_fini(void) {
    wrmsr(IA32_FMASK_MSR, 0);
    wrmsr(IA32_LSTAR_MSR, NULL);
}


enum status syscall_register(uint64_t syscall_number, syscall_f function) {
    enum status status;

    if (syscall_number > Syscall_Max) {
        return Error_Range;
    }

    lock_acquire_writer(&Syscall_Table_Lock);

    if (Syscall_Table[syscall_number].function != NULL) {
        status = Error_Present;
        goto out;
    }

    Syscall_Table[syscall_number].function = function;

out:
    lock_release_writer(&Syscall_Table_Lock);
    return status;
}


void syscall_unregister(uint64_t syscall_number) {
    if (syscall_number > Syscall_Max) {
        return;
    }

    lock_acquire_writer(&Syscall_Table_Lock);
    Syscall_Table[syscall_number].function = NULL;
    lock_release_writer(&Syscall_Table_Lock);
}
