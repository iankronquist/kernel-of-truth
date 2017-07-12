#include <arch/x64/boot.h>
#include <arch/x64/asm.h>

#define Boot_VGA_Window_Start 0xb8000
#define CPU_Flags_ID 0x00200000
#define CPUId_Leaf_Feature_Info   0x1
#define CPUId_Leaf_Processor_Info 0x80000001
#define CPUId_EDX_Long_Mode (1 << 29)
#define CPUId_EDX_Time_Step_Counter (1 << 4)
#define CPUId_EDX_Not_Executable_Pages (1 << 20)
#define CPUId_EDX_Syscall (1 << 11)
#define MSR_IA32_EFER_Long_Mode 0x100
#define MSR_IA32_EFER_Execute_Disable 0x800
.section .text

# MB header.
.align MB_Alignment
.long MB_Magic
.long MB_Flags
.long MB_Checksum
.section .bss, "aw", @nobits
.align Page_Small

.global init_pl4
init_pl4:
	.skip Page_Small

.global init_pl3
init_pl3:
	.skip Page_Small

.global init_pl2
init_pl2:
	.skip Page_Small

.global init_pl1_a
init_pl1_a:
	.skip Page_Small

.global init_pl1_b
init_pl1_b:
	.skip Page_Small

.global init_pl1_c
init_pl1_c:
	.skip Page_Small


.global _init_stack_top
init_stack:
	.skip (16 * KB)
_init_stack_top:

.section .text

.code32

Function(_start)
	# Initialize stack pointer.
	movl $_init_stack_top, %esp

	# Save Multiboot info table physical address.
	# Push a 0 in front of it so when we do a 64 bit pop later we don't
	# underflow the stack.
	pushl $0
	pushl %ebx
	movl %esp, %ebp



.compat_check_multiboot:
	# Is this a multiboot compatible bootloader?
	cmpl $Multiboot_Register_Magic, %eax
	je .compat_check_cpuid
	call boot_vga_clear
	# If not, print an error message, and hang.
	movl $Boot_Bad_Multiboot_Magic_String, %edi
	jmp .bad_boot


.compat_check_cpuid:

	call boot_vga_clear

	# Is cpuid supported? It is if the eax register is not 0.
	call boot_cpuid_available
	testl %eax, %eax
	jnz .compat_check_long_mode
	movl $Boot_No_CPUId, %edi
	jmp .bad_boot

.compat_check_long_mode:
	movl $CPUId_Leaf_Processor_Info, %eax
	cpuid

	movl %edx, %edi
	andl $CPUId_EDX_Long_Mode, %edi
	jnz .compat_check_nx
	movl $Boot_No_Long_Mode_String, %edi
	jmp .bad_boot

.compat_check_nx:
	movl %edx, %edi
	andl $CPUId_EDX_Not_Executable_Pages, %edi
	jnz .compat_check_syscall
	movl $Boot_No_NX_Bit_String, %edi
	jmp .bad_boot

.compat_check_syscall:
	movl %edx, %edi
	andl $CPUId_EDX_Syscall, %edi
	jnz .compat_check_tsc
	movl $Boot_No_Syscall_String, %edi
	jmp .bad_boot

.compat_check_tsc:
	movl $CPUId_Leaf_Feature_Info, %eax
	cpuid

	movl %edx, %edi
	andl $CPUId_EDX_Time_Step_Counter, %edi
	jnz .enable_paging
	movl $Boot_No_RDTSC_String, %edi
	jmp .bad_boot


.enable_paging:
	call boot_map_page_table

	# Enable PAE.
	movl %cr4, %eax
	orl $CR4_PAE, %eax
	movl %eax, %cr4


	# Enable long mode and the No-Execute bit.
	movl $IA32_EFER_MSR, %ecx
	rdmsr
	orl $(MSR_IA32_EFER_Long_Mode | MSR_IA32_EFER_Execute_Disable), %eax
	wrmsr

	# Enable paging (with write protection) and enter long mode.
	# Note that we are still in a 32 bit code segment.
	movl %cr0, %eax
	orl $(CR0_Paging | CR0_Write_Protect), %eax
	movl %eax, %cr0


	lgdt Boot_GDT_Register
	ljmp $Segment_Kernel_Code, $long_mode

.code64
long_mode:
	# Clear upper 32 bits of stack pointer.
	movl %esp, %esp

	# Load kernel data segment.
	movw $Segment_Kernel_Data, %cx
	movw %cx, %ds
	movw %cx, %es
	movw %cx, %ss

	pop %rdi
	call boot_loader_main
	jmp boot_halt
.bad_boot:
	call boot_vga_log
	jmp boot_halt

.code32

Function(boot_map_page_table)
	movl $init_pl4, %edi
	movl %edi, %cr3

	movl $(init_pl3 + (Page_User | Page_Write | Page_Present)), init_pl4

	movl $(init_pl2 + (Page_User | Page_Write | Page_Present)), init_pl3

	movl $(init_pl1_a + (Page_Write | Page_Present)), init_pl2
	movl $(init_pl1_b + (Page_Write | Page_Present)), init_pl2 + 8
	movl $(init_pl1_c + (Page_Write | Page_Present)), init_pl2 + 16

	# Map first 4 MB except for the NULL page.
	movl $(init_pl1_a + 8), %edi
	movl $(0x1000 | Page_Present | Page_Write), %esi
	movl $0x1ff, %ecx
1:
	movl %esi, (%edi)
	addl $8, %edi
	addl $Page_Small, %esi
	loop 1b

	addl $Page_Small, %esi
	# Map next pl1
	movl $(init_pl1_b + 8), %edi
	movl $0x200, %ecx
1:
	movl %esi, (%edi)
	addl $8, %edi
	addl $Page_Small, %esi
	loop 1b

	# Map next pl1
	movl $(init_pl1_c + 8), %edi
	movl $0x200, %ecx
1:
	movl %esi, (%edi)
	addl $8, %edi
	addl $Page_Small, %esi
	loop 1b


	ret


Function(boot_vga_clear)
	movl $(80*24), %ecx
	movl $Boot_VGA_Window_Start, %eax
1:
	movl $0, (%eax)
	addl $4, %eax
	loop 1b
	ret

# void boot_vga_log(char *edi);
Function(boot_vga_log)
	movl $Boot_VGA_Window_Start, %ebx
.boot_vga_log_loop:
	movl (%edi), %eax
	testb %al, %al
	jz .out
	movb $0x0f, %ah
	movw %ax, (%ebx)
	addl $2, %ebx
	inc %edi
	jmp .boot_vga_log_loop
.out:
	ret

Function(boot_halt)
	cli
	hlt
	pause
	jmp boot_halt


Function(boot_cpuid_available)
	# Save original flags.
	pushf
	# Save them again. We'll manipulate these ones.
	pushf
	# Flip the ID bit.
	xorl $CPU_Flags_ID, (%esp)
	# Restore the manipulated flags. If cpuid is supported the bit will be set.
	popf
	# Move flags to eax.
	pushf
	pop %eax
	# Mask everything but the id bit.
	andl $CPU_Flags_ID, %eax
	# Restore original flags.
	popf
	ret

.section .data

Boot_Bad_Multiboot_Magic_String:
.asciz "Bad multiboot magic. Incompatible bootloader."

Boot_No_CPUId:
.asciz "cpuid instruction not supported. Incompatible hardware."

Boot_No_Long_Mode_String:
.asciz "64 bit not supported. Incompatible hardware."

Boot_No_RDTSC_String:
.asciz "Time Step Counter not supported. Incompatible hardware."

Boot_No_NX_Bit_String:
.asciz "Non-executable pages not supported. Incompatible hardware."

Boot_No_Syscall_String:
.asciz "Syscall instruction not supported. Incompatible hardware."

.section .data.kernel
.align Page_Small
.global _binary_build_truth_x64_elf64_start
.global _binary_build_truth_x64_elf64_end
_binary_build_truth_x64_elf64_start:
.incbin "build/truth.x64.elf64"
_binary_build_truth_x64_elf64_end:

