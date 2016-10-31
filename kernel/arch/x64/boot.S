#include "boot.h"

.section .text

	# Multiboot header.
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
init_pl1_b:
	.skip Page_Small
.global slab_higher_half
slab_higher_half:
	.skip Page_Small
.global slab_lower_half
slab_lower_half:
	.skip Page_Small
.global init_physical_allocator_vector
init_physical_allocator_vector:
	.skip Page_Small
init_stack:
	.skip (16 * KB)
_init_stack_top:

.section .text

.global _start
.type _start, @function
.code32
_start:
	# Initialize stack pointer.
	movl $_init_stack_top, %esp

	# Finish installing the kernel stack into the Task Switch Segment.
	movl %esp, Tss + 4
	movl $0, Tss + 8

	# Finish installing the TSS into the GDT
	movl $Tss, %ecx
	movw %cx, GDT + TSS_Segment + 2
	shrl $16, %ecx
	movb %cl, GDT + TSS_Segment + 4
	shrl $8, %ecx
	movb %cl, GDT + TSS_Segment + 7
	movl $0, GDT + TSS_Segment + 8

	# We got our multiboot information in various registers.
	pushl $0
	pushl %ebx

	movl $init_pl4, %edi
	movl %edi, %cr3

	# Page-Map Level 4.
	movl $(init_pl3 + (Page_User | Page_Write | Page_Present)), init_pl4

	# Page Directory Pointer Table.
	movl $(init_pl2 + (Page_User | Page_Write | Page_Present)), init_pl3

	# Page Directory (no user-space access here).
	movl $(init_pl1_a + (Page_Write | Page_Present)), init_pl2
	movl $(init_pl1_b + (Page_Write | Page_Present)), init_pl2 + 8

	# Page Table (identity map the first 4 MiB, except NULL).
	# TODO: This is insecure as it doesn't restrict write & execute access to
	#       the code kernel code & variables appropriately.
	movl $(init_pl1_a + 8), %edi
	movl $(0x1000 | Page_Present | Page_Write), %esi
	movl $1023, %ecx
1:
	movl %esi, (%edi)
	addl $Page_Small, %esi
	addl $8, %edi
	loop 1b

	# Fractal mapping.
	movl $(init_pl4 + (Page_Write | Page_Present)), init_pl4 + 511 * 8

	# Enable PAE.
	movl %cr4, %eax
	orl $CR4_PAE, %eax
	movl %eax, %cr4

	# Enable long mode and the No-Execute bit.
	movl $IA32_EFER_MSR, %ecx
	rdmsr
	orl $0x900, %eax
	wrmsr

	# Enable paging (with write protection) and enter long mode (still 32-bit)
	movl %cr0, %eax
	orl $(CR0_Paging | CR0_Write_Protect), %eax
	movl %eax, %cr0
paging:

	# Load the Global Descriptor Table pointer register.
	subl $6, %esp
	movw (GDT_Size - 1), %cx
	movw %cx, 0(%esp)
	movl $GDT, %ecx
	movl %ecx, 2(%esp)
	lgdt 0(%esp)
	addl $6, %esp

	# Now use the 64-bit code segment, and we are in full 64-bit mode.
	ljmp $Code_Segment, $long_mode

.code64
long_mode:
	# Clear upper 32 bits of stack pointer.
	mov %esp, %esp

	# Load kernel data segment.
	movw $Data_Segment, %cx
	movw %cx, %ds
	movw %cx, %es
	movw %cx, %ss

	# Switch the task switch segment register to the task switch segment (0x28).
	movw $(TSS_Segment | RPL), %cx
	ltr %cx

	# Switch to the thread local fs and gs segments.
	movw $(User_Data_Segment | RPL), %cx
	movw %cx, %fs
	movw %cx, %gs

	# Multiboot information structure pointer.
	pop %rdi
	call kernel_main

	jmp halt

.global halt
.type halt, @function
halt:
	cli
	hlt
	jmp halt

.global cpu_time
.type cpu_time, @function
cpu_time:
    rdtsc
    shlq $32, %rdx
    orq %rax, %rdx
    ret


.global get_base_pointer
.type get_base_pointer, @function
get_base_pointer:
    mov %rbp, %rax
    ret
