#include <arch/x64/boot.h>

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
	movw %cx, GDT + Segment_TSS + 2
	shrl $16, %ecx
	movb %cl, GDT + Segment_TSS + 4
	shrl $8, %ecx
	movb %cl, GDT + Segment_TSS + 7
	movl $0, GDT + Segment_TSS + 8

	# We got our multiboot information in various registers.
	pushl $0
	pushl %ebx

	movl $init_pl4, %edi
	movl %edi, %cr3

	movl $(init_pl3 + (Page_User | Page_Write | Page_Present)), init_pl4

	movl $(init_pl2 + (Page_User | Page_Write | Page_Present)), init_pl3

	movl $(init_pl1_a + (Page_Write | Page_Present)), init_pl2
	movl $(init_pl1_b + (Page_Write | Page_Present)), init_pl2 + 8

	# Page Table (identity map the first 1 MiB, except NULL).
	movl $(init_pl1_a + 8), %edi
	movl $(0x1000 | Page_Present | Page_Write), %esi
	movl $Page_Execute_Disable_High, %edx
	movl $0xff, %ecx
1:
	movl %esi, (%edi)
	addl $4, %edi
	movl %edx, (%edi)
	addl $Page_Small, %esi
	addl $4, %edi
	loop 1b


	# Map kernel text section
	andl $(Memory_Writable_Mask), %esi
	movl $0, %edx
	movl $__text_end, %ecx
	subl $__text_start, %ecx
	shrl $12, %ecx
1:
	movl %esi, (%edi)
	addl $4, %edi
	movl %edx, (%edi)
	addl $Page_Small, %esi
	addl $4, %edi
	loop 1b

	# Map kernel data section
	orl $(Memory_Writable), %esi
	movl $Page_Execute_Disable_High, %edx
	movl $__data_end, %ecx
	subl $__data_start, %ecx
	shrl $12, %ecx
1:
	movl %esi, (%edi)
	addl $4, %edi
	movl %edx, (%edi)
	addl $Page_Small, %esi
	addl $4, %edi
	loop 1b

	# Map kernel rodata section
	andl $(Memory_Writable_Mask), %esi
	movl $Page_Execute_Disable_High, %edx
	movl $__rodata_end, %ecx
	subl $__rodata_start, %ecx
	shrl $12, %ecx
1:
	movl %esi, (%edi)
	addl $4, %edi
	movl %edx, (%edi)
	addl $Page_Small, %esi
	addl $4, %edi
	loop 1b

	# Map kernel bss section
	orl $(Memory_Writable), %esi
	movl $Page_Execute_Disable_High, %edx
	movl $__bss_end, %ecx
	subl $__bss_start, %ecx
	shrl $12, %ecx
1:
	movl %esi, (%edi)
	addl $4, %edi
	movl %edx, (%edi)
	addl $Page_Small, %esi
	addl $4, %edi
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
	ljmp $Segment_Kernel_Code, $long_mode

.code64
long_mode:
	# Clear upper 32 bits of stack pointer.
	mov %esp, %esp

	# Load kernel data segment.
	movw $Segment_Kernel_Data, %cx
	movw %cx, %ds
	movw %cx, %es
	movw %cx, %ss

	# Switch the task switch segment register to the task switch segment.
	movw $(Segment_TSS | Segment_RPL), %cx
	ltr %cx

	# Switch to the thread local fs and gs segments.
	movw $(Segment_User_Data | Segment_RPL), %cx
	movw %cx, %fs
	movw %cx, %gs

	# Multiboot information structure pointer.
	pop %rdi
	call kernel_main

	jmp halt

.global halt
.type halt, @function
halt:
	hlt
	jmp halt

.global cpu_time
.type cpu_time, @function
cpu_time:
    rdtsc
    shlq $32, %rdx
    orq %rax, %rdx
    ret
