#include <arch/x64/cpu.h>
#include <truth/log.h>


void supervisor_memory_protection_init(void) {
    uint32_t eax, ebx, ecx, edx;
    eax = 7;
    ecx = 0;
    cpuid(&eax, &ebx, &ecx, &edx);
    if (ebx & CPUID_SMEP) {
        log(Log_Info, "SMEP enabled.");
        cpu_cr4_set_bit(CPU_CR4_SMEP_BIT);
    }

    if (ebx & CPUID_SMAP) {
        log(Log_Info, "SMAP enabled.");
        cpu_cr4_set_bit(CPU_CR4_SMAP_BIT);
    }
}

void memory_init(void) {
    supervisor_memory_protection_init();
}

void memory_user_access_enable(void) {
    cpu_flags_set_ac();
}

void memory_user_access_disable(void) {
    cpu_flags_clear_ac();
}
