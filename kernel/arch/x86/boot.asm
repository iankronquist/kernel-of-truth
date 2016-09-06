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


global _start
_start:
	; Enter protected mode
	mov eax, cr0
	or al, 1
	mov cr0, eax

	mov esp, bootstrap_stack_top
	push ebx ; Multiboot header pointer

	extern bootstrap_paging
	call bootstrap_paging

	mov eax, bootstrap_pdpt
	mov cr3, eax

	xor		edx,	edx
	mov		eax,	(1 << 11)
	mov		ecx,	0xc0000080
	wrmsr

	; Use PAE paging.
	mov		eax,	cr4
	or		eax,	(1 << 5)
	mov		cr4,	eax

	; Enable paging and ring 0 write protection.
	mov		eax,	cr0
	or		eax,	(1 << 31) | (1 << 16)
global paging_on
paging_on:
	mov		cr0,	eax

	extern kernel_main
	call kernel_main

	cli
	hlt
Lhang:
	jmp Lhang

section .bss

align 4096
global bootstrap_page_dir
bootstrap_page_dir:
resb 4096

global bootstrap_page_table
bootstrap_page_table:
resb 4096

global bootstrap_stack_canary
bootstrap_stack_canary:
resb 4096

global bootstrap_stack_bottom
bootstrap_stack_bottom:
resb 16384 ; 16 kb
global bootstrap_stack_top
bootstrap_stack_top:

align 24
global bootstrap_pdpt
bootstrap_pdpt:
resb 256
