MBALIGN    equ 1<<0
MEMINFO  equ 1<<1
FLAGS    equ MBALIGN | MEMINFO
MAGIC    equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM

section .bootstrap_stack
stack_bottom:
resb 16384 ; 16 kb
stack_top:


section .text
global _start
_start:
	; Enter protected mode
	mov eax, cr0
	or al, 1
	mov cr0, eax

	mov esp, stack_top

	extern kernel_main
	call kernel_main

	cli
	hlt
Lhang:
	jmp Lhang
