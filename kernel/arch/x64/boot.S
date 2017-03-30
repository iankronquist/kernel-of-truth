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

.global init_pl1
init_pl1:
	.skip Page_Small

.global slab_higher_half
slab_higher_half:
       .skip Page_Small

.global init_physical_allocator_vector
init_physical_allocator_vector:
       .skip Page_Small

.global _init_stack_top
init_stack:
	.skip (16 * KB)
_init_stack_top:

.section .text

.global _start
.type _start, @function
.code32
_start:
	# Initialize stack pointer.
	movl $virt_to_phys(_init_stack_top), %esp

	# Finish installing the kernel stack into the Task Switch Segment.
	; movl %esp, virt_to_phys(Tss) + 4
	; movl $Higher_Half_High, virt_to_phys(Tss) + 8

	# Finish installing the TSS into the GDT
	; movl $virt_to_phys(Tss), %ecx
	; movw %cx, virt_to_phys(GDT) + Segment_TSS + 2
	; shrl $16, %ecx
	; movb %cl, virt_to_phys(GDT) + Segment_TSS + 4
	; shrl $8, %ecx
	; movb %cl, virt_to_phys(GDT) + Segment_TSS + 7
	; movl $Higher_Half_High, virt_to_phys(GDT) + Segment_TSS + 8

	# We got our multiboot information in various registers.
	pushl $0
	pushl %ebx

	movl $virt_to_phys(init_pl4), %edi
	movl %edi, %cr3

	movl $(virt_to_phys(init_pl3) + (Page_User | Page_Write | Page_Present)), virt_to_phys(init_pl4)
	movl $(virt_to_phys(init_pl3) + (Page_User | Page_Write | Page_Present)), virt_to_phys(init_pl4) + 511 * 8

	movl $(virt_to_phys(init_pl2) + (Page_User | Page_Write | Page_Present)), virt_to_phys(init_pl3)
	movl $(virt_to_phys(init_pl2) + (Page_User | Page_Write | Page_Present)), virt_to_phys(init_pl3) + 510 * 8

	movl $(virt_to_phys(init_pl1) + (Page_Write | Page_Present)), virt_to_phys(init_pl2)

	# Fractal mapping.
	movl $(virt_to_phys(init_pl4) + (Page_Write | Page_Present)), virt_to_phys(init_pl4) + Kernel_Fractal_Page_Table_Index * 8
	movl $Page_Execute_Disable_High, virt_to_phys(init_pl4) + (Kernel_Fractal_Page_Table_Index * 8) + 4


	# Page Table (identity map the first 1 MiB, except NULL).
	movl $(virt_to_phys(init_pl1) + 8), %edi
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
	movl $virt_to_phys(__text_end), %ecx
	subl $virt_to_phys(__text_start), %ecx
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
	movl $virt_to_phys(__data_end), %ecx
	subl $virt_to_phys(__data_start), %ecx
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
	movl $virt_to_phys(__rodata_end), %ecx
	subl $virt_to_phys(__rodata_start), %ecx
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
	movl $virt_to_phys(__bss_end), %ecx
	subl $virt_to_phys(__bss_start), %ecx
	shrl $12, %ecx
1:
	movl %esi, (%edi)
	addl $4, %edi
	movl %edx, (%edi)
	addl $Page_Small, %esi
	addl $4, %edi
	loop 1b

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
	lgdt virt_to_phys(Physical_GDT_Register)

	# Now use the 64-bit code segment, and we are in full 64-bit mode.
	ljmp $Segment_Kernel_Code, $virt_to_phys(long_mode)

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
	#movw $(Segment_TSS | Segment_RPL), %cx
	#ltr %cx

	# Switch to the thread local fs and gs segments.
	#movw $(Segment_User_Data | Segment_RPL), %cx
	#movw %cx, %fs
	#movw %cx, %gs

	add $Kernel_Space_Start, %rsp
	mov $higher_half, %rcx
	jmp *%rcx

higher_half:
	# Load the Global Descriptor Table pointer register again.
	lgdt GDT_Register



	# Multiboot information structure pointer.
	pop %rdi
	call kernel_main

	jmp halt

.global halt
.type halt, @function
halt:
	hlt
	pause
	jmp halt

.global cpu_sleep_state
.type cpu_sleep_state, @function
cpu_sleep_state:
	hlt
    ret


.global cpu_time
.type cpu_time, @function
cpu_time:
    rdtsc
    shlq $32, %rdx
    orq %rax, %rdx
    ret

.global read_rip
.type read_rip, @function
read_rip:
	pop %rax
	push %rax
	ret
