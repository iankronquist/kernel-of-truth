#include "boot.h"

.section .text

	# Multiboot header.
	.align MB_ALIGN
	.long MB_MAGIC
	.long MB_FLAGS
	.long MB_CHECKSUM

.section .bss, "aw", @nobits
	.align SMALL_PAGE
.global init_pl4
init_pl4:
	.skip SMALL_PAGE
.global init_pl3
init_pl3:
	.skip SMALL_PAGE
.global init_pl2
init_pl2:
	.skip SMALL_PAGE
.global init_pl1_a
init_pl1_a:
	.skip SMALL_PAGE
init_pl1_b:
	.skip SMALL_PAGE
.global slab_higher_half
slab_higher_half:
	.skip SMALL_PAGE
.global slab_lower_half
slab_lower_half:
	.skip SMALL_PAGE
.global init_physical_allocator_vector
init_physical_allocator_vector:
	.skip SMALL_PAGE
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
	movw %cx, Gdt + TSS_SEGMENT + 2
	shrl $16, %ecx
	movb %cl, Gdt + TSS_SEGMENT + 4
	shrl $8, %ecx
	movb %cl, Gdt + TSS_SEGMENT + 7
	movl $0, Gdt + TSS_SEGMENT + 8

	# We got our multiboot information in various registers.
	pushl $0
	pushl %ebx

	movl $init_pl4, %edi
	movl %edi, %cr3

	# Page-Map Level 4.
	movl $(init_pl3 + (PAGE_USER | PAGE_WRITE | PAGE_PRESENT)), init_pl4

	# Page Directory Pointer Table.
	movl $(init_pl2 + (PAGE_USER | PAGE_WRITE | PAGE_PRESENT)), init_pl3

	# Page Directory (no user-space access here).
	movl $(init_pl1_a + (PAGE_WRITE | PAGE_PRESENT)), init_pl2
	movl $(init_pl1_b + (PAGE_WRITE | PAGE_PRESENT)), init_pl2 + 8

	# Page Table (identity map the first 4 MiB, except NULL).
	# TODO: This is insecure as it doesn't restrict write & execute access to
	#       the code kernel code & variables appropriately.
	movl $(init_pl1_a + 8), %edi
	movl $(0x1000 | PAGE_PRESENT | PAGE_WRITE), %esi
	movl $1023, %ecx
1:
	movl %esi, (%edi)
	addl $SMALL_PAGE, %esi
	addl $8, %edi
	loop 1b

	# Fractal mapping.
	movl $(init_pl4 + (PAGE_WRITE | PAGE_PRESENT)), init_pl4 + 511 * 8

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
	orl $(CR0_PAGING | CR0_WRITE_PROTECT), %eax
	movl %eax, %cr0
paging:

	# Load the Global Descriptor Table pointer register.
	subl $6, %esp
	movw (Gdt_Size - 1), %cx
	movw %cx, 0(%esp)
	movl $Gdt, %ecx
	movl %ecx, 2(%esp)
	lgdt 0(%esp)
	addl $6, %esp

	# Now use the 64-bit code segment, and we are in full 64-bit mode.
	ljmp $CODE_SEGMENT, $long_mode

.code64
long_mode:
	# Clear upper 32 bits of stack pointer.
	mov %esp, %esp

	# Load kernel data segment.
	movw $DATA_SEGMENT, %cx
	movw %cx, %ds
	movw %cx, %es
	movw %cx, %ss

	# Switch the task switch segment register to the task switch segment (0x28).
	movw $(TSS_SEGMENT | RPL), %cx
	ltr %cx

	# Switch to the thread local fs and gs segments.
	movw $(USER_DATA_SEGMENT | RPL), %cx
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
