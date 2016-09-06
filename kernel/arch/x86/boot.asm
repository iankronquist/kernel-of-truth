extern bootstrap_paging
extern kernel_main

global _start
global bootstrap_pdpt
global bootstrap_page_dir
global bootstrap_page_table
global bootstrap_pdpt
global bootstrap_stack_canary
global bootstrap_stack_bottom
global bootstrap_stack_top

; Define symbols used in the multiboot header.
MBALIGN  equ 1 << 0
MEMINFO  equ 1 << 1
FLAGS    equ MBALIGN | MEMINFO
MAGIC    equ 0x1badb002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM


_start:
	; Enter protected mode
	mov eax, cr0
	or al, 1
	mov cr0, eax

	; Set up stack.
	mov esp, bootstrap_stack_top
	; Save multiboot header pointer.
	push ebx

	; Set up bootstrap page tables.
	call bootstrap_paging

	mov eax, bootstrap_pdpt
	mov cr3, eax

	; Use PAE paging.
	mov		eax,	cr4
	or		eax,	(1 << 5)
	mov		cr4,	eax

	; Enable paging and ring 0 write protection.
	mov		eax,	cr0
	or		eax,	(1 << 31) | (1 << 16)
	mov		cr0,	eax

	; Enter kernel_main, passing the multiboot header pointer.
	call kernel_main

	; If we ever unexpectedly return from kernel_main, clear interrupts and
	; panic.
	cli
	hlt
hang:
	jmp hang

section .bss

align 4096
bootstrap_page_dir:
resb 4096

bootstrap_page_table:
resb 4096

bootstrap_stack_canary:
resb 4096

bootstrap_stack_bottom:
resb 16384 ; 16 kb
bootstrap_stack_top:

align 24
bootstrap_pdpt:
resb 256
