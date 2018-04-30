#pragma once
#include <truth/types.h>


enum vmcs_field {
    Virtual_Processor_ID            = 0x00000000,
    Posted_Intr_NV                  = 0x00000002,
    Guest_ES_Selector               = 0x00000800,
    Guest_CS_Selector               = 0x00000802,
    Guest_SS_Selector               = 0x00000804,
    Guest_DS_Selector               = 0x00000806,
    Guest_FS_Selector               = 0x00000808,
    Guest_GS_Selector               = 0x0000080a,
    Guest_LDTR_Selector             = 0x0000080c,
    Guest_TR_Selector               = 0x0000080e,
    Guest_Intr_Status               = 0x00000810,
    Guest_PML_Index                 = 0x00000812,
    Host_ES_Selector                = 0x00000c00,
    Host_CS_Selector                = 0x00000c02,
    Host_SS_Selector                = 0x00000c04,
    Host_DS_Selector                = 0x00000c06,
    Host_FS_Selector                = 0x00000c08,
    Host_GS_Selector                = 0x00000c0a,
    Host_TR_Selector                = 0x00000c0c,
    IO_Bitmap_A                     = 0x00002000,
    IO_Bitmap_A_High                = 0x00002001,
    IO_Bitmap_B                     = 0x00002002,
    IO_Bitmap_B_High                = 0x00002003,
    MSR_Bitmap                      = 0x00002004,
    MSR_Bitmap_High                 = 0x00002005,
    VM_Exit_MSR_Store_addr          = 0x00002006,
    VM_Exit_MSR_Store_addr_High     = 0x00002007,
    VM_Exit_MSR_Load_addr           = 0x00002008,
    VM_Exit_MSR_Load_addr_High      = 0x00002009,
    VM_Entry_MSR_Load_addr          = 0x0000200a,
    VM_Entry_MSR_Load_addr_High     = 0x0000200b,
    PML_Address                     = 0x0000200e,
    PML_Address_High                = 0x0000200f,
    TSC_Offset                      = 0x00002010,
    TSC_Offset_High                 = 0x00002011,
    Virtual_APIC_Page_addr          = 0x00002012,
    Virtual_APIC_Page_addr_High     = 0x00002013,
    APIC_Access_Addr                = 0x00002014,
    APIC_Access_Addr_High           = 0x00002015,
    Posted_Intr_Desc_addr           = 0x00002016,
    Posted_Intr_Desc_addr_High      = 0x00002017,
    VM_Function_Control             = 0x00002018,
    VM_Function_Control_High        = 0x00002019,
    EPT_Pointer                     = 0x0000201a,
    EPT_Pointer_High                = 0x0000201b,
    EOI_Exit_Bitmap0                = 0x0000201c,
    EOI_Exit_Bitmap0_High           = 0x0000201d,
    EOI_Exit_Bitmap1                = 0x0000201e,
    EOI_Exit_Bitmap1_High           = 0x0000201f,
    EOI_Exit_Bitmap2                = 0x00002020,
    EOI_Exit_Bitmap2_High           = 0x00002021,
    EOI_Exit_Bitmap3                = 0x00002022,
    EOI_Exit_Bitmap3_High           = 0x00002023,
    EPTP_List_Address               = 0x00002024,
    EPTP_List_Address_High          = 0x00002025,
    VMRead_Bitmap                   = 0x00002026,
    VMWrite_Bitmap                  = 0x00002028,
    XSS_Exit_Bitmap                 = 0x0000202C,
    XSS_Exit_Bitmap_High            = 0x0000202D,
    TSC_Multiplier                  = 0x00002032,
    TSC_Multiplier_High             = 0x00002033,
    Guest_Physical_Address          = 0x00002400,
    Guest_Physical_Address_High     = 0x00002401,
    VMCS_Link_Pointer               = 0x00002800,
    VMCS_Link_Pointer_High          = 0x00002801,
    Guest_IA32_DebugCtl             = 0x00002802,
    Guest_IA32_DebugCtl_High        = 0x00002803,
    Guest_IA32_PAT                  = 0x00002804,
    Guest_IA32_PAT_High             = 0x00002805,
    Guest_IA32_EFER                 = 0x00002806,
    Guest_IA32_EFER_High            = 0x00002807,
    Guest_IA32_Perf_Global_Ctrl     = 0x00002808,
    Guest_IA32_Perf_Global_Ctrl_High= 0x00002809,
    Guest_PDPTR0                    = 0x0000280a,
    Guest_PDPTR0_High               = 0x0000280b,
    Guest_PDPTR1                    = 0x0000280c,
    Guest_PDPTR1_High               = 0x0000280d,
    Guest_PDPTR2                    = 0x0000280e,
    Guest_PDPTR2_High               = 0x0000280f,
    Guest_PDPTR3                    = 0x00002810,
    Guest_PDPTR3_High               = 0x00002811,
    Guest_BNDCFGS                   = 0x00002812,
    Guest_BNDCFGS_High              = 0x00002813,
    Host_IA32_PAT                   = 0x00002c00,
    Host_IA32_PAT_High              = 0x00002c01,
    Host_IA32_EFER                  = 0x00002c02,
    Host_IA32_EFER_High             = 0x00002c03,
    Host_IA32_Perf_GLOBAL_Ctrl      = 0x00002c04,
    Host_IA32_Perf_GLOBAL_Ctrl_High = 0x00002c05,
    Pin_Based_VM_Exec_Control       = 0x00004000,
    CPU_Based_VM_Exec_Control       = 0x00004002,
    Exception_Bitmap                = 0x00004004,
    Page_Fault_Error_Code_Mask      = 0x00004006,
    Page_Fault_Error_Code_Match     = 0x00004008,
    CR3_Target_Count                = 0x0000400a,
    VM_Exit_Controls                = 0x0000400c,
    VM_Exit_MSR_Store_Count         = 0x0000400e,
    VM_Exit_MSR_Load_Count          = 0x00004010,
    VM_Entry_Controls               = 0x00004012,
    VM_Entry_MSR_Load_Count         = 0x00004014,
    VM_Entry_Intr_Info_Field        = 0x00004016,
    VM_Entry_Exception_Error_Code   = 0x00004018,
    VM_Entry_Instruction_Len        = 0x0000401a,
    TPR_Threshold                   = 0x0000401c,
    Secondary_VM_Exec_Control       = 0x0000401e,
    PLE_Gap                         = 0x00004020,
    PLE_Window                      = 0x00004022,
    VM_Instruction_Error            = 0x00004400,
    VM_Exit_Reason                  = 0x00004402,
    VM_Exit_Intr_Info               = 0x00004404,
    VM_Exit_Intr_Error_Code         = 0x00004406,
    IDT_Vectoring_Info_Field        = 0x00004408,
    IDT_Vectoring_Error_Code        = 0x0000440a,
    VM_Exit_Instruction_Len         = 0x0000440c,
    VMX_Instruction_Info            = 0x0000440e,
    Guest_ES_Limit                  = 0x00004800,
    Guest_CS_Limit                  = 0x00004802,
    Guest_SS_Limit                  = 0x00004804,
    Guest_DS_Limit                  = 0x00004806,
    Guest_FS_Limit                  = 0x00004808,
    Guest_GS_Limit                  = 0x0000480a,
    Guest_LDTR_Limit                = 0x0000480c,
    Guest_TR_Limit                  = 0x0000480e,
    Guest_GDTR_Limit                = 0x00004810,
    Guest_IDTR_Limit                = 0x00004812,
    Guest_ES_AR_Bytes               = 0x00004814,
    Guest_CS_AR_Bytes               = 0x00004816,
    Guest_SS_AR_Bytes               = 0x00004818,
    Guest_DS_AR_Bytes               = 0x0000481a,
    Guest_FS_AR_Bytes               = 0x0000481c,
    Guest_GS_AR_Bytes               = 0x0000481e,
    Guest_LDTR_AR_Bytes             = 0x00004820,
    Guest_TR_AR_Bytes               = 0x00004822,
    Guest_Interruptibility_Info     = 0x00004824,
    Guest_Activity_State            = 0X00004826,
    Guest_SysEnter_CS               = 0x0000482A,
    VMX_Preemption_Timer_Value      = 0x0000482E,
    Host_IA32_SysEnter_CS           = 0x00004c00,
    CR0_Guest_Host_Mask             = 0x00006000,
    CR4_Guest_Host_Mask             = 0x00006002,
    CR0_Read_Shadow                 = 0x00006004,
    CR4_Read_Shadow                 = 0x00006006,
    CR3_Target_Value0               = 0x00006008,
    CR3_Target_Value1               = 0x0000600a,
    CR3_Target_Value2               = 0x0000600c,
    CR3_Target_Value3               = 0x0000600e,
    Exit_Qualification              = 0x00006400,
    Guest_Linear_Address            = 0x0000640a,
    Guest_CR0                       = 0x00006800,
    Guest_CR3                       = 0x00006802,
    Guest_CR4                       = 0x00006804,
    Guest_ES_Base                   = 0x00006806,
    Guest_CS_Base                   = 0x00006808,
    Guest_SS_Base                   = 0x0000680a,
    Guest_DS_Base                   = 0x0000680c,
    Guest_FS_Base                   = 0x0000680e,
    Guest_GS_Base                   = 0x00006810,
    Guest_LDTR_Base                 = 0x00006812,
    Guest_TR_Base                   = 0x00006814,
    Guest_GDTR_Base                 = 0x00006816,
    Guest_IDTR_Base                 = 0x00006818,
    Guest_DR7                       = 0x0000681a,
    Guest_RSP                       = 0x0000681c,
    Guest_RIP                       = 0x0000681e,
    Guest_RFLAGS                    = 0x00006820,
    Guest_Pending_Dbg_Exceptions    = 0x00006822,
    Guest_SysEnter_ESP              = 0x00006824,
    Guest_SysEnter_EIP              = 0x00006826,
    Host_CR0                        = 0x00006c00,
    Host_CR3                        = 0x00006c02,
    Host_CR4                        = 0x00006c04,
    Host_FS_Base                    = 0x00006c06,
    Host_GS_Base                    = 0x00006c08,
    Host_TR_Base                    = 0x00006c0a,
    Host_GDTR_Base                  = 0x00006c0c,
    Host_IDTR_Base                  = 0x00006c0e,
    Host_IA32_SysEnter_ESP          = 0x00006c10,
    Host_IA32_SysEnter_EIP          = 0x00006c12,
    Host_RSP                        = 0x00006c14,
    Host_RIP                        = 0x00006c16,
};

struct invvpid_descriptor {
    uint64_t linear_address;
    uint8_t reserved[48];
    uint16_t vpid;
};

#define RPL_Mask 0x3

// For GDT & IDT.
struct cpu_table_descriptor {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));


struct segment_descriptor {
    union {
        struct {
            uint16_t limit_low;
            uint16_t base_low;
            uint8_t base_middle;
            uint8_t access;
            uint8_t granularity;
            uint8_t base_high;
            uint32_t base_highest;
            uint32_t reserved0;
        } __attribute__((packed));
        uint64_t bits;
    };
};


extern int invvpid(uint64_t, struct invvpid_descriptor*);
extern int vmclear(phys_addr *addr);
extern int vmptrld(uint64_t*);
extern void vmptrst(uint64_t*);
extern int vmread(enum vmcs_field field, uint64_t *value);
extern int vmwrite(enum vmcs_field field, uint64_t value);
extern int vmlaunch(void);
extern int vmlaunch_extended(void);
extern int vmresume(void);
extern int vmxoff(void);
extern int vmxon(phys_addr*);
extern void vm_entry_point_wrapper(void);

int vmx_init(void);
extern uint64_t read_cr0(void);
extern void write_cr0(uint64_t);

#ifndef read_cr3
extern uint64_t read_cr3(void);
#endif
//extern void write_cr3(uint64_t);

extern uint64_t read_cr4(void);
extern void write_cr4(uint64_t);

extern uint64_t read_dr7(void);

extern void disable_A20_interrupts(void);

extern uint64_t read_rbp(void);
extern uint64_t read_rsp(void);
extern uint64_t read_rflags(void);

extern uint16_t read_ss(void);
extern uint16_t read_cs(void);
extern uint16_t read_ds(void);
extern uint16_t read_es(void);
extern uint16_t read_fs(void);
extern uint16_t read_gs(void);

extern void sidt(struct cpu_table_descriptor *);
extern void lidt(struct cpu_table_descriptor *);
extern void sgdt(struct cpu_table_descriptor *);
extern void lgdt(struct cpu_table_descriptor *);
extern uint16_t lsl(uint16_t segment);
extern uint16_t str(void);
extern uint16_t sldt(void);
// 'hype'
#define VMX_Magic 0x68797065
#define VMX_Command_Unload 0x1

static inline void hypercall(int command) {
    __asm__ volatile ("mov %1, %%ebx; mov %0, %%eax; vmcall" : : "b"(VMX_Magic), "a"(command));
}

struct vmcs {
    uint32_t revision;
    uint32_t abort;
    uint8_t data[];
};

enum vmcs_abort_reason {
    VMCS_Abort_Ok = 0,
    VMCS_Abort_Guest_MSR = 1,
    VMCS_Abort_Host_PDPTE = 2,
    VMCS_Abort_Corrupt = 3,
    VMCS_Abort_Host_MSR = 4,
    VMCS_Abort_VM_Exit_Check = 5,
    VMCS_Abort_Long_Mode_Inconcistent = 6,
};


int vmx_in_host(void);

#define MSR_IA32_VMX_Basic              0x00000480
#define MSR_IA32_VMX_PinBased_Ctls      0x00000481
#define MSR_IA32_VMX_ProcBased_Ctls     0x00000482
#define MSR_IA32_VMX_Exit_Ctls          0x00000483
#define MSR_IA32_VMX_Entry_Ctls         0x00000484
#define MSR_IA32_VMX_Misc               0x00000485
#define MSR_IA32_VMX_CR0_Fixed0         0x00000486
#define MSR_IA32_VMX_CR0_Fixed1         0x00000487
#define MSR_IA32_VMX_CR4_Fixed0         0x00000488
#define MSR_IA32_VMX_CR4_Fixed1         0x00000489
#define MSR_IA32_VMX_VMCS_Enum          0x0000048a
#define MSR_IA32_VMX_ProcBased_Ctls2    0x0000048b
#define MSR_IA32_VMX_EPT_VPID_Cap       0x0000048c
#define MSR_IA32_VMX_True_PinBased_Ctls  0x0000048d
#define MSR_IA32_VMX_True_PROCBased_Ctls 0x0000048e
#define MSR_IA32_VMX_True_Exit_Ctls      0x0000048f
#define MSR_IA32_VMX_True_Entry_Ctls     0x00000490
#define MSR_IA32_VMX_VMFUNC             0x00000491

#define VMX_Basic_VMCS_Size_Shift 32
#define VMX_Basic_True_Ctls (1ULL << 55)
#define VMX_Basic_64 0x0001000000000000LLU
#define VMX_Basic_Mem_Type_Shift 50
#define VMX_Basic_Mem_Type_Mask 0x003c000000000000LLU
#define VMX_Basic_Mem_Type_WB 6LLU
#define VMX_Basic_InOut 0x0040000000000000LLU

#define MSR_IA32_DebugCtlMSR        0x000001d9

#define CPU_Based_Virtual_Intr_PENDING          0x00000004
#define CPU_Based_USE_TSC_OffsetING             0x00000008
#define CPU_Based_HLT_Exiting                   0x00000080
#define CPU_Based_INVLPG_Exiting                0x00000200
#define CPU_Based_MWAIT_Exiting                 0x00000400
#define CPU_Based_RDPMC_Exiting                 0x00000800
#define CPU_Based_RDTSC_Exiting                 0x00001000
#define CPU_Based_CR3_Load_Exiting        0x00008000
#define CPU_Based_CR3_Store_Exiting        0x00010000
#define CPU_Based_CR8_Load_Exiting              0x00080000
#define CPU_Based_CR8_Store_Exiting             0x00100000
#define CPU_Based_TPR_Shadow                    0x00200000
#define CPU_Based_Virtual_NMI_Pending        0x00400000
#define CPU_Based_MOV_DR_Exiting                0x00800000
#define CPU_Based_Uncond_IO_Exiting             0x01000000
#define CPU_Based_Use_IO_Bitmaps                0x02000000
#define CPU_Based_Monitor_Trap_Flag             0x08000000
#define CPU_Based_Use_MSR_Bitmaps               0x10000000
#define CPU_Based_Monitor_Exiting               0x20000000
#define CPU_Based_Pause_Exiting                 0x40000000
#define CPU_Based_Activate_Secondary_Controls   0x80000000

#define CPU_Based_Alwayson_Without_True_MSR    0x0401e172

#define Secondary_Exec_Virtualize_APIC_Accesses 0x00000001
#define Secondary_Exec_Enable_EPT               0x00000002
#define Secondary_Exec_Desc            0x00000004
#define Secondary_Exec_RDTSCP            0x00000008
#define Secondary_Exec_Virtualize_X2APIC_Mode   0x00000010
#define Secondary_Exec_Enable_VPID              0x00000020
#define Secondary_Exec_WBINVD_Exiting        0x00000040
#define Secondary_Exec_Unrestricted_Guest    0x00000080
#define Secondary_Exec_APIC_REGISTER_Virt       0x00000100
#define Secondary_Exec_Virtual_Intr_Delivery    0x00000200
#define Secondary_Exec_PAUSE_LOOP_Exiting    0x00000400
#define Secondary_Exec_RDRAND_Exiting        0x00000800
#define Secondary_Exec_Enable_INVPCID        0x00001000
#define Secondary_Exec_Enable_VMFUNC            0x00002000
#define Secondary_Exec_Shadow_VMCS              0x00004000
#define Secondary_Exec_RDSEED_Exiting        0x00010000
#define Secondary_Exec_Enable_PML               0x00020000
#define Secondary_Exec_XSaves            0x00100000
#define Secondary_Exec_TSC_Scaling              0x02000000

#define Pin_Based_EXT_Intr_Mask                 0x00000001
#define Pin_Based_NMI_Exiting                   0x00000008
#define Pin_Based_Virtual_NMIS                  0x00000020
#define Pin_Based_VMX_Preemption_Timer          0x00000040
#define Pin_Based_Posted_Intr                   0x00000080

#define Pin_Based_Alwayson_Without_True_MSR    0x00000016

#define VM_Exit_Save_Debug_Controls             0x00000004
#define VM_Exit_Host_Addr_Space_Size            0x00000200
#define VM_Exit_Load_IA32_Perf_GlobaL_Ctrl      0x00001000
#define VM_Exit_Ack_Intr_On_Exit                0x00008000
#define VM_Exit_Save_IA32_PAT            0x00040000
#define VM_Exit_Load_IA32_PAT            0x00080000
#define VM_Exit_Save_IA32_EFER                  0x00100000
#define VM_Exit_Load_IA32_EFER                  0x00200000
#define VM_Exit_Save_VMX_Preemption_Timer       0x00400000
#define VM_Exit_Clear_BndCFGS                   0x00800000

#define VM_Exit_Alwayson_Without_True_MSR    0x00036dff

#define VM_Entry_Load_Debug_Controls            0x00000004
#define VM_Entry_IA32E_Mode                     0x00000200
#define VM_Entry_SMM                            0x00000400
#define VM_Entry_Deact_Dual_Monitor             0x00000800
#define VM_Entry_Load_IA32_Perf_Global_Ctrl     0x00002000
#define VM_Entry_Load_IA32_PAT            0x00004000
#define VM_Entry_Load_IA32_EFER                 0x00008000
#define VM_Entry_Load_BndCFGS                   0x00010000

#define VM_Entry_Alwayson_Without_True_MSR    0x000011ff

#define VMX_Misc_Preemption_Timer_Rate_Mask    0x0000001f
#define VMX_Misc_Save_EFER_LMA            0x00000020
#define VMX_Misc_Activity_HLT            0x00000040

#define VMX_VMFUNC_EPTP_Switching               0x00000001
#define VMFUNC_EPTP_Entries  512

#define MSR_IA32_SysEnter_CS        0x00000174
#define MSR_IA32_SysEnter_ESP        0x00000175
#define MSR_IA32_SysEnter_EIP        0x00000176


#define MSR_EFER        0xc0000080
#define MSR_STAR        0xc0000081
#define MSR_LSTAR        0xc0000082
#define MSR_CSTAR        0xc0000083
#define MSR_Syscall_Mask    0xc0000084
#define MSR_FS_Base        0xc0000100
#define MSR_GS_Base        0xc0000101
#define MSR_Kernel_GS_Base    0xc0000102
#define MSR_TSC_Aux        0xc0000103

#define MSR_IA32_Perf_Status        0x00000198
#define MSR_IA32_CR_PAT            0x00000277
#define MSR_IA32_Perf_Ctl        0x00000199

#define MSR_IA32_SysEnter_CS        0x00000174
#define MSR_IA32_SysEnter_ESP        0x00000175
#define MSR_IA32_SysEnter_EIP        0x00000176

#define Feature_Control_Locked                (1<<0)
#define Feature_Control_VMXON_Enabled_Inside_SMX    (1<<1)
#define Feature_Control_VMXON_Enabled_Outside_SMX    (1<<2)
#define Feature_Control_LMCE                (1<<20)


#define MSR_IA32_Feature_Control        0x0000003a
