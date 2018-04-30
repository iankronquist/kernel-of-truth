#include <truth/slab.h>
#include <truth/string.h>
#include <truth/panic.h>
#include <truth/memory.h>
#include <truth/log.h>
#include <arch/x64/vmx.h>
#include <arch/x64/paging.h>


#define VMX_Query_Present 0x2
#define Host_Stack_Size (16 * 1024)
#define VMX_Invalid_Revision (1u << 31)

// Updated in vmx.S
bool in_vmx_host = true;

int vmx_in_host(void) {
    return in_vmx_host;
}

static inline void cpuid(uint32_t *eax, uint32_t *ebx, uint32_t *ecx,
        uint32_t *edx) {
    __asm__ volatile ("cpuid" :
            "=a"(*eax),
            "=b"(*ebx),
            "=c"(*ecx),
            "=d"(*edx)
            : "0" (*eax)
            :);
}

int _vmwrite(char *name, enum vmcs_field field, uint64_t value) {
    logf(Log_Error, "vmwrite: %s %x %lx\n", name, field, value);
    return vmwrite(field, value);
}

void wrmsr(uint32_t msr, uint32_t low, uint32_t high);
void rdmsr(uint32_t msr, uint32_t *low, uint32_t *high);
void wrmsr(uint32_t msr, uint32_t low, uint32_t high) {
    __asm__ volatile ("wrmsr" : :
            "d"(high),
            "a"(low),
            "c"(msr)
            :);
}

void _wrmsrl(char *name, uint32_t msr, uint64_t val) {
    uint32_t low, high;
    logf(Log_Error, "wrmsr: %s %x %lx\n", name, msr, val);
    low = val & 0xffffffff;
    high = (val>>32) & 0xffffffff;
    wrmsr(msr, low, high);
}

void _rdmsr(char *name, uint32_t msr, uint32_t *low, uint32_t *high) {
    uint32_t h,l;
    __asm__ volatile ("rdmsr" :
            "=d"(h),
            "=a"(l)
            : "c" (msr)
            :);
    *low = l;
    *high = h;
    logf(Log_Debug, "rdmsr %s %x %x %x\n", name, msr, l, h);
}

uint64_t _rdmsrl(char *name, uint32_t msr) {
    uint32_t low, high;
    _rdmsr(name, msr, &low, &high);
    return ((uint64_t)high << 32) | low;
}

#define wrmsrl(msr, val) _wrmsrl(#msr, msr, val)
#define rdmsr(msr, h,l) _rdmsr(#msr, msr, h, l)
#define rdmsrl(msr) _rdmsrl(#msr, msr)
#define vmwrite(x, y) _vmwrite(#x, x, y)

struct load_state {
    uint64_t cr3;
    uint64_t status;
    uint64_t failed_cpu;
    atomic_uint loaded_cpu_count;
};

static uint64_t original_cr0;
static uint64_t original_cr4;
static uint32_t *vmcs;
static uint32_t *vmx_region;
static void *host_stack;
static uint8_t msr_bitmap[Page_Small] = {0};
static atomic_uint vm_exit_count = 0;
static bool vm_successfully_launched = false;
static bool vmx_on = false;


static bool is_vmx_supported(void) {
    unsigned int eax, ebx, ecx, edx;
    eax = 1;
    cpuid(&eax, &ebx, &ecx, &edx);
    return ecx & (1 << 5);
}


static void unload(void) {
    vmxoff();

}


// When a VM exit occurs, the processor will start executing code the
// vm_entry_point_wrapper, which saves the processor state and calls this.
void vm_entry_point(uint32_t magic, uint32_t command) {
    uint64_t field, rip, length;

    atomic_fetch_add_explicit(&vm_exit_count, 1, memory_order_acquire);
    log(Log_Debug, "VM exit");
    // See 27.7.
    logf(Log_Debug, "abort: %d\n", vmcs[1]);

    // See 27.2.1 and appendix C.
    vmread(VM_Exit_Reason, &field);
    logf(Log_Debug, "reason: %ld\n", field);

    vmread(Exit_Qualification, &field);
    logf(Log_Debug, "qualification: %ld\n", field);

    vmread(Guest_RIP, &rip);
    logf(Log_Debug, "Guest RIP: %lx\n", rip);

    vmread(Guest_RSP, &field);
    logf(Log_Debug, "Guest_RSP: %lx\n", field);
 
    vmread(VM_Instruction_Error, &field);
    logf(Log_Debug, "VM Instruction Error: %ld\n", field);

    vmread(VM_Exit_Instruction_Len, &length);
    logf(Log_Debug, "VM Instruction Length: %ld\n", length);

    vmread(VM_Exit_Intr_Info, &field);
    logf(Log_Debug, "VM Exit Interrupt Info: %ld\n", field);

    vmread(VM_Exit_Intr_Error_Code, &field);
    logf(Log_Debug, "VM Exit Interrupt Error Code: %ld\n", field);

    if (magic == VMX_Magic) {
        switch (command) {
            case VMX_Command_Unload:
                unload();
                break;
            default:
                break;
        };
    }
}


// Read a value from an MSR and split it into bits which must be high, and bits
// which must be low.
static uint32_t _adjust_msr(char *name, uint32_t msr, uint32_t value) {
    uint32_t msr_low;
    uint32_t msr_high;
    uint32_t orig = value;

    _rdmsr(name, msr, &msr_low, &msr_high);

    value = msr_low |  (msr_high & value);

    logf(Log_Debug, "MSR %x value: %x -> %x\n", msr, orig, value);
    return value;
}
#define adjust_msr(msr, val) _adjust_msr(#msr, msr, val)


// Current CR3 actually belongs to insmod when this is called and will be destroyed when it exits. Get the page global directory.
uint64_t get_host_cr3(void) {
    return read_cr3();
}


// Ancient x86 segmentation sucks. GDT descriptors have bits packed all over
// and fields slapped at the end from the 16 bit to 32 bit to 64 bit
// architecture transitions.
uint64_t unscramble_segment_base(void *gdt, uint16_t segment, uint32_t *access_rights) {

    uint64_t base;
    struct segment_descriptor *desc;
    desc = (struct segment_descriptor *)(gdt + (segment & ~RPL_Mask));
    logf(Log_Debug, "Unscrambling segment %hx of GDT %p\n", segment, gdt);
    if (segment == 0) {
        // Set unusable bit.
        *access_rights = 1<<16;
        return 0;
    }

    base = (((uint64_t)desc->base_high << 24) |
            ((uint64_t)desc->base_middle << 16) |
            desc->base_low
           ) & 0xffffffff;
    base |= ((desc->access & 0x80) == 0) ?
        ((uint64_t)desc->base_highest << 32) : 0ull;
    *access_rights = (desc->bits >> 40) & 0xf0ff;
    //*access_rights = ((desc->granularity << 8) | desc->access) & 0xf0ff;
    logf(Log_Debug, "Base: %lx\n", base);
    logf(Log_Debug, "Access Rights: %x\n", *access_rights);

    return base;
}


// When a VMX instruction is sets either the Z bit or the C bit. I save these
// in an integer I return as a status code to C code. This functions checks
// that status code and then optionally consults the VM instruction error VMCS
// field.
// See section 30.4 volume 3, table 30-1 for a table of the error numbers.
void report_launch_error(uint64_t status) {
    uint64_t value = 0;
    if (status & 1) {
        logf(Log_Info, "Invalid VMCS\n");
    } else if (status & 256) {
        vmread(VM_Instruction_Error, &value);
        logf(Log_Info, "Valid VMCS with error number %lx\n", value);
    } else {
        logf(Log_Info, "Bad status\n");
    }
}

// Initialize a VMCS.
void vmcs_init(uint64_t guest_rip, uint64_t unused(guest_rsp), void *host_stack,
        uint64_t host_stack_size, uint64_t host_cr3) {
    uint64_t cr;
    uint64_t msr;
    uint64_t base;
    uint32_t segment_access_rights;
    uint16_t segment;
    void *gdt;
    struct cpu_table_descriptor gdtr;
    struct cpu_table_descriptor idtr;

    sgdt(&gdtr);
    gdt = (void *)gdtr.base;

    // Set an invalid link pointer since we don't have a shadow VMCS.
    vmwrite(VMCS_Link_Pointer, ~0ll);

    // Set MSR bitmap.
    vmwrite(MSR_Bitmap, virt_to_phys(msr_bitmap));

    // Set guest CPU, pin, execution, VM Exit, and VM Entry controls.
    // These settings should be very similar to SimpleVisor.
    // See chapter 24 sections 24.6-24.9.
    vmwrite(Secondary_VM_Exec_Control,
            adjust_msr(MSR_IA32_VMX_ProcBased_Ctls2,
                Secondary_Exec_RDTSCP | Secondary_Exec_XSaves));

    //vmwrite(Pin_Based_VM_Exec_Control,
    //        adjust_msr(MSR_IA32_VMX_True_PinBased_Ctls, 0));

    vmwrite(Pin_Based_VM_Exec_Control,
            adjust_msr(MSR_IA32_VMX_PinBased_Ctls, 0));

    //vmwrite(CPU_Based_VM_Exec_Control,
    //        adjust_msr(MSR_IA32_VMX_True_ProcBased_Ctls,
    //            CPU_Based_Activate_Secondary_Controls));
    vmwrite(CPU_Based_VM_Exec_Control,
            adjust_msr(MSR_IA32_VMX_ProcBased_Ctls,
                CPU_Based_Activate_Secondary_Controls));


    //vmwrite(VM_Exit_Controls,
    //        adjust_msr(MSR_IA32_VMX_True_Exit_Ctls,
    //            VM_Exit_Ack_Intr_On_Exit |
    //            VM_Exit_Host_Addr_Space_Size));
    vmwrite(VM_Exit_Controls,
            adjust_msr(MSR_IA32_VMX_Exit_Ctls,
                VM_Exit_Ack_Intr_On_Exit |
                VM_Exit_Host_Addr_Space_Size));

    //vmwrite(VM_Entry_Controls,
    //        adjust_msr(MSR_IA32_VMX_True_Entry_Ctls,
    //            VM_Entry_IA32E_Mode));



    vmwrite(VM_Entry_Controls,
            adjust_msr(MSR_IA32_VMX_Entry_Ctls,
                VM_Entry_IA32E_Mode));

    // Set guest segment registers, and host segment registers where
    // applicable, and the task register.
    // FIXME: Very repetitive. Refactor.
    segment = read_cs();
    vmwrite(Guest_CS_Selector, segment);
    vmwrite(Guest_CS_Limit, lsl(segment));
    vmwrite(Guest_CS_Base, unscramble_segment_base(gdt, segment, &segment_access_rights));
    vmwrite(Guest_CS_AR_Bytes, segment_access_rights);
    vmwrite(Host_CS_Selector, segment & ~RPL_Mask);

    segment = read_ss();
    vmwrite(Guest_SS_Selector, segment);
    vmwrite(Guest_SS_Limit, lsl(segment));
    vmwrite(Guest_SS_Base, unscramble_segment_base(gdt, segment, &segment_access_rights));
    vmwrite(Guest_SS_AR_Bytes, segment_access_rights);
    vmwrite(Host_SS_Selector, segment & ~RPL_Mask);


    segment = read_ds();
    vmwrite(Guest_DS_Selector, segment);
    vmwrite(Guest_DS_Limit, lsl(segment));
    vmwrite(Guest_DS_Base, unscramble_segment_base(gdt, segment, &segment_access_rights));
    vmwrite(Guest_DS_AR_Bytes, segment_access_rights);
    vmwrite(Host_DS_Selector, segment & ~RPL_Mask);

    segment = read_es();
    vmwrite(Guest_ES_Selector, segment);
    vmwrite(Guest_ES_Limit, lsl(segment));
    vmwrite(Guest_ES_Base, unscramble_segment_base(gdt, segment, &segment_access_rights));
    vmwrite(Guest_ES_AR_Bytes, segment_access_rights);
    vmwrite(Host_ES_Selector, segment & ~RPL_Mask);

    segment = read_fs();
    vmwrite(Guest_FS_Selector, segment);
    vmwrite(Guest_FS_Limit, lsl(segment));
    base = unscramble_segment_base(gdt, segment, &segment_access_rights);
    vmwrite(Guest_FS_AR_Bytes, segment_access_rights);
    vmwrite(Guest_FS_Base, base);
    vmwrite(Host_FS_Base, base);
    vmwrite(Host_FS_Selector, segment & ~RPL_Mask);

    segment = read_gs();
    vmwrite(Guest_GS_Selector, segment);
    vmwrite(Guest_GS_Limit, lsl(segment));
    base = unscramble_segment_base(gdt, segment, &segment_access_rights);
    vmwrite(Guest_GS_AR_Bytes, segment_access_rights);
    vmwrite(Guest_GS_Base, base);
    vmwrite(Host_GS_Base, base);
    vmwrite(Host_GS_Selector, segment & ~RPL_Mask);

    segment = str();
    vmwrite(Guest_TR_Selector, segment);
    vmwrite(Guest_TR_Limit, lsl(segment));
    base = unscramble_segment_base(gdt, segment, &segment_access_rights);
    vmwrite(Guest_TR_Base, base);
    vmwrite(Host_TR_Base, base);
    vmwrite(Guest_TR_AR_Bytes, segment_access_rights);
    vmwrite(Host_TR_Selector, segment & ~RPL_Mask);

    segment = sldt();
    vmwrite(Guest_LDTR_Selector, segment);
    vmwrite(Guest_LDTR_Limit, lsl(segment));
    base = unscramble_segment_base(gdt, segment, &segment_access_rights);
    vmwrite(Guest_LDTR_Base, base);
    //vmwrite(Host_LDTR_Base, base);
    vmwrite(Guest_LDTR_AR_Bytes, segment_access_rights);
    //vmwrite(Host_LDTR_Selector, segment & ~RPL_Mask);


    // Set guest and host control registers.
    logf(Log_Debug, "CR0\n");
    cr = read_cr0();
    vmwrite(CR0_Read_Shadow, cr);
    vmwrite(Host_CR0, cr);
    vmwrite(Guest_CR0, cr);

    logf(Log_Debug, "CR4\n");
    cr = read_cr4();
    vmwrite(CR4_Read_Shadow, cr);
    vmwrite(Host_CR4, cr);
    vmwrite(Guest_CR4, cr);

    logf(Log_Debug, "CR3\n");
    vmwrite(Host_CR3, host_cr3);
    vmwrite(Guest_CR3, read_cr3());

    logf(Log_Debug, "Debug controls\n");
    msr = rdmsrl(MSR_IA32_DebugCtlMSR);
    vmwrite(Guest_IA32_DebugCtl, msr);
    vmwrite(Guest_DR7, read_dr7());


    logf(Log_Debug, "Guest RSP, RIP, RFLAGS\n");
    vmwrite(Guest_RFLAGS, read_rflags());
    //vmwrite(Guest_RSP, guest_rsp);
    vmwrite(Guest_RIP, guest_rip);

    logf(Log_Debug, "HOST RIP, RSP\n");
    vmwrite(Host_RIP, (uint64_t)vm_entry_point_wrapper);
    vmwrite(Host_RSP, (uint64_t)host_stack + host_stack_size);

    logf(Log_Debug, "Host IDTR, GDTR bases\n");
    sidt(&idtr);
    vmwrite(Host_IDTR_Base, idtr.base);
    vmwrite(Guest_IDTR_Base, idtr.base);
    vmwrite(Guest_IDTR_Limit, idtr.limit);

    vmwrite(Host_GDTR_Base, gdtr.base);
    vmwrite(Guest_GDTR_Base, gdtr.base);
    vmwrite(Guest_GDTR_Limit, gdtr.limit);

    // Not in SimpleVisor but they seem awfully necessary.
    // Still fails if these are commented out.
    msr = rdmsrl(MSR_IA32_SysEnter_CS);
    vmwrite(Guest_SysEnter_CS, msr);

    msr = rdmsrl(MSR_IA32_SysEnter_ESP);
    vmwrite(Guest_SysEnter_ESP, msr);

    msr = rdmsrl(MSR_IA32_SysEnter_EIP);
    vmwrite(Guest_SysEnter_EIP, msr);

    msr = rdmsrl(MSR_IA32_Perf_Ctl);
    vmwrite(Guest_IA32_Perf_Global_Ctrl_High, msr);

    msr = rdmsrl(MSR_IA32_CR_PAT);
    vmwrite(Guest_IA32_PAT_High, msr);

    msr = rdmsrl(MSR_EFER);
    vmwrite(Guest_IA32_EFER_High, msr);

}


// Unload the hypervisor on a CPU.
static int unload_cpu(void) {

    // If the hypervisor successfully launched, send a hypercall telling it to
    // unload. Otherwise, just unload it directly.
    if (vm_successfully_launched) {
        logf(Log_Debug, "Hypercall\n");
        hypercall(VMX_Command_Unload);
    } else if (vmx_on) {
        unload();
    }

    slab_free(Host_Stack_Size, host_stack);
    slab_free(Page_Small, vmx_region);
    slab_free(Page_Small, vmcs);
    return 0;
}


uint32_t get_revision_id(void) {
    uint64_t msr;

    static uint32_t vmcs_revision = VMX_Invalid_Revision;

    if (vmcs_revision == VMX_Invalid_Revision) {
        logf(Log_Debug, "Reading MSR_IA32_VMX_Basic\n");
        msr = rdmsrl(MSR_IA32_VMX_Basic);
        vmcs_revision = msr & 0xffffffff;
    }

    return vmcs_revision;
}


// Per CPU load. Reads a bunch of CPU settings which could be read just once
// and cached but CPUs are fast and I am lazy.
static int load_cpu(void *arg) {
    int status = 0;
    uint32_t revision_id;
    uint64_t cr0;
    uint64_t cr4;
    uint64_t msr;
    uint64_t guest_rsp;
    //uint32_t *vmcs;
    //uint32_t *vmx_region;
    //void *host_stack;
    phys_addr phys_vmcs;
    phys_addr phys_vmx_region;
    struct load_state *load_state = arg;

    logf(Log_Debug, "Read rsp\n");
    guest_rsp = read_rsp();
    revision_id = get_revision_id();
    logf(Log_Debug, "Revision id 0x%x\n", revision_id);

    logf(Log_Debug, "host stack alloc\n");
    host_stack = slab_alloc(Host_Stack_Size, Memory_Writable);
    if(host_stack == NULL) {
        load_state->status = Error_No_Memory;
        logf(Log_Error, "Not enough memory for host stack\n");
        goto err;
    }

    logf(Log_Debug, "vmx region alloc\n");
    vmx_region = slab_alloc(Page_Small, Memory_Writable);
    logf(Log_Debug, "v0\n");
    if(vmx_region == NULL) {
        logf(Log_Debug, "v1\n");
        load_state->status = Error_No_Memory;
        logf(Log_Error, "Not enough memory for VMX region\n");
        slab_free(Host_Stack_Size, host_stack);
        goto err;
    }
    logf(Log_Debug, "v2\n");
    phys_vmx_region = virt_to_phys(vmx_region);
    logf(Log_Debug, "v3\n");
    memset(vmx_region, 0, Page_Small);
    logf(Log_Debug, "v4\n");
    *vmx_region = revision_id;
    logf(Log_Debug, "vmx region %p %lx\n", vmx_region, phys_vmx_region);

    logf(Log_Debug, "vmcs alloc\n");
    vmcs = slab_alloc(Page_Small, Memory_Writable);
    if(vmcs == NULL) {
        load_state->status = Error_No_Memory;
        logf(Log_Error, "Not enough memory for VMCS region\n");
        slab_free(Host_Stack_Size, host_stack);
        slab_free(Page_Small, vmx_region);
        goto err;
    }
    phys_vmcs = virt_to_phys(vmcs);
    logf(Log_Debug, "vmcs alloc %p %lx\n", vmcs, phys_vmcs);
    memset(vmcs, 0, Page_Small);
    *vmcs = revision_id;

    logf(Log_Debug, "cr0 set\n");
    cr0 = read_cr0();
    original_cr0 = cr0;
    msr = rdmsrl(MSR_IA32_VMX_CR0_Fixed0);
    logf(Log_Debug, "fixed %lx\n", msr);
    cr0 |= msr;
    msr = rdmsrl(MSR_IA32_VMX_CR0_Fixed1);
    logf(Log_Debug, "clear %lx (should not be zero!)\n", msr);
    assert(msr != 0);
    cr0 &= msr;
    logf(Log_Debug, "writing cr0 %lx %lx\n", original_cr0, cr0);
    write_cr0(cr0);
    logf(Log_Debug, "written cr0\n");

    logf(Log_Debug, "cr4 set\n");
    cr4 = read_cr4();
    original_cr4 = cr4;
    msr = rdmsrl(MSR_IA32_VMX_CR4_Fixed0);
    cr4 |= msr;
    msr = rdmsrl(MSR_IA32_VMX_CR4_Fixed1);
    cr4 &= msr;
    write_cr4(cr4);

    logf(Log_Debug, "Read feature control\n");
    msr = rdmsrl(MSR_IA32_Feature_Control);

    // Kind of perfunctory -- the lock bit should be set by the BIOS or in
    // early boot.
    logf(Log_Debug, "Check lock bit\n");
    if ((msr & Feature_Control_Locked) == 0) {
        logf(Log_Debug, "Enable VMX outside SMX and set lock bit\n");
        msr = rdmsrl(MSR_IA32_Feature_Control);
        msr |= Feature_Control_VMXON_Enabled_Outside_SMX |
            Feature_Control_Locked;
        wrmsrl(MSR_IA32_Feature_Control, msr);
    } else if ((msr & Feature_Control_VMXON_Enabled_Outside_SMX) == 0) {
        logf(Log_Error, "Lock bit set without enabling VMX\n");
        goto err;
    }

    logf(Log_Debug, "vmxon stack: 0x%p phys: 0x%lx virt: 0x%p\n", &phys_vmx_region, phys_vmx_region, vmx_region);
    memset(vmx_region, 0, Page_Small);
    vmx_region[0] = revision_id;
    logf(Log_Debug, "vmx region magic: expected: 0x%x actual: 0x%x\n", revision_id, *vmx_region);

    /*
    logf(Log_Debug, "Disabling A20 line\n");
    disable_A20_interrupts();
    logf(Log_Debug, "Disabled A20 line\n");
    */

    assert((phys_vmx_region & 0xfff) == 0);
    status = vmxon(&phys_vmx_region);
    if (status != 0) {
        logf(Log_Debug, "vmxon failed! %x\n", status);
        goto err;
    }
    vmx_on = true;

    logf(Log_Debug, "vmclear p: %lx v: %p\n", phys_vmcs, &phys_vmcs);
    status = vmclear(&phys_vmcs);
    if (status != 0) {
        logf(Log_Debug, "vmclear failed!\n");
        goto err;
    }
    logf(Log_Debug, "vmptrld\n");
    status = vmptrld(&phys_vmcs);
    if (status != 0) {
        logf(Log_Debug, "vmptrld failed!\n");
        goto err;
    }

    logf(Log_Debug, "vmcs_init\n");
    vmcs_init((uint64_t)&&out, guest_rsp, host_stack, Host_Stack_Size,
            load_state->cr3);

    // Set up per CPU variables -- VMCSes, VMX regions, stacks etc.
    logf(Log_Debug, "vmlaunch\n");
    status = vmlaunch_extended();
    if (status != 0) {
        logf(Log_Debug, "VMLaunch failed! %d\n", status);
        report_launch_error(status);
        goto err;
    }

out:
    logf(Log_Debug, "In VM\n");
    atomic_fetch_add_explicit(&load_state->loaded_cpu_count, 1, memory_order_acquire);
err:
    load_state->status = -1;

    return status;
}


// Execution starts here when the module is inserted.
int vmx_init(void) {
    int err;
    int cpu = 0;
    int status = 0;
    struct load_state load_state;
    load_state.cr3 = get_host_cr3();
    load_state.status = 0;
    load_state.failed_cpu = -1;

    log(Log_Debug, "hello vmx");

    atomic_store_explicit(&load_state.loaded_cpu_count, 0, memory_order_release);

    if(!is_vmx_supported()) {
        logf(Log_Error, "Virtual machine extensions aren't supported by this "
                "CPU or were disabled in the BIOS\n");
    }


    // Run VM bootstrap serially on all cores.

    logf(Log_Debug, "Loading on core %d\n", cpu);
    err = load_cpu(&load_state);
    if(err != 0) {
        logf(Log_Error, "Task was interrupted\n");
        status = err;
    }

    if(atomic_load_explicit(&load_state.loaded_cpu_count, memory_order_acquire) != 1) {
        logf(Log_Error, "Couldn't load all CPUs\n");
        status = load_state.status;
    } else {
        vm_successfully_launched = true;
        logf(Log_Debug, "Hypervisor loaded\n");
    }

    return status;
}


void vmx_fini(void) {
    int err;
    int cpu = 0;
    // Run VM teardown serially on all CPUs.
    logf(Log_Debug, "Unloading on core %d\n", cpu);
    err = unload_cpu();
    if(err != 0) {
        logf(Log_Error, "Task was interrupted\n");
    }

    logf(Log_Debug, "Restore control registers\n");
    write_cr0(original_cr0);
    write_cr4(original_cr4);

    logf(Log_Debug, "VM exit count %d\n", atomic_load_explicit(&vm_exit_count, memory_order_acquire));
    logf(Log_Info, "Hypervisor unloaded\n");
}
