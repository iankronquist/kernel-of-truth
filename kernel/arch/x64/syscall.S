#include <arch/x64/asm.h>

#include <truth/process.h>

.extern syscall_entrypoint

# Recall that the syscall instruction:
# 1. Points the RIP at the value in the IA32_LSTAR_MSR, in this case this function.
# 2. Puts rflags in r11.
# 3. Masks rflags with the IA32_FMASK_MSR.
# Also recall that the base of the kernel GS register holds the address of the
# current thread structure.
# To enter kernel space from userspace we must:
# 1. Swap the userspace GS base with the kernel GS base.
# 2. Exchange the userspace stack pointer with the kernel stack pointer.
# 3. Save all of the callee saved registers
# 4. Call the syscall_entrypoint.
# To return back to userspace, reverse the process.
Function(_syscall_entrypoint):
    swapgs
    xchgq %gs:(Thread_Current_Stack_GS_Offset), %rsp
    push %r11
    Push_Caller_Saved
    call syscall_entrypoint
    Pop_Caller_Saved
    pop %r11
    xchgq %gs:(Thread_Current_Stack_GS_Offset), %rsp
    swapgs
    sysret
