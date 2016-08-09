.section .boot

// allocate stack 
.section .bootstrap_stack
.section .text
.align 4
stack_bottom:
.space 16384 // 16kb
stack_top:

// entrypoint
.global _start
_start:
	mov sp, #stack_top

	ldr r3, =kernel_main
	blx r3
.hang:
	wfe
	b .hang
