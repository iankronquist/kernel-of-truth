global _start
extern kernel_main
extern kernel_start
extern kernel_end

LINK_ADDR equ 0x40000000

section .multiboot

	;  Add the multiboot header, else the bootloader won't recognise us.
	MBALIGN    equ 1<<0
	MEMINFO  equ 1<<1
	FLAGS    equ MBALIGN | MEMINFO
	MAGIC    equ 0x1BADB002
	CHECKSUM equ -(MAGIC + FLAGS)

	mboot_header:
	align 4
		dd MAGIC
		dd FLAGS
		dd CHECKSUM

section .text

	;  At this point paging is not setup yet,
	;  we must use position-independent code and PHYSICAL addresses.*/

	_start:
		;  Physical address of pagetab.
		mov dword edi, pagetab + LINK_ADDR
		;  First address to map is address 0.
		mov esi, 0
		;  Map 1023 pages. (We reserve the 1024th for VGA text buffer)
		mov ecx, 1023

	fill_page_table:
		;  Clear flags (lower 12 bits).
		and esi, 0xFFFFF000
		;  Size of page is 4096 bytes.
		add esi, 4096
		;  Size of entries in pagetab is 4 bytes.
		add edi, 4

		;  Map physical address in esi to esi+0xC0000000 as "present, writable".
		or esi, 3
		mov [edi], esi

		cmp esi, (kernel_end + LINK_ADDR)
		jge done
		;  Add the next entry if we haven't finished.
		loop fill_page_table

	done:
		;  Map VGA video memory to 0xC03FF000 as "present, writable".
		mov dword [pagetab + LINK_ADDR + 1023*4], 0x000B8003

	;  Now we need to both map it at 0xC0000000 and identity map it.
	;  The reason for the identity mapping is
	;  that the CPU will fail to fetch the next instruction after paging is enabled.
		
	setup:
		;  Map the page table to both virtual addresses 0x00000000 and 0xC0000000.
		mov dword [pagedir + LINK_ADDR + 0], (pagetab + LINK_ADDR + 0x0003)
		mov dword [pagedir + LINK_ADDR + 768*4], (pagetab + LINK_ADDR + 0x0003)
		mov dword [pagedir + LINK_ADDR + 1023*4], (pagedir + LINK_ADDR + 0x0003)
		;  Set cr3 to the address of the pagedir
		mov ecx, (pagedir + LINK_ADDR)
		mov cr3, ecx
		mov ecx, cr0
		or ecx, 0x80000000
		mov cr0, ecx
		;  Long jump to higher half.
		lea ecx, [higher_half]
		jmp ecx

	higher_half:
		;  Unmap the identity mapping. It should not be needed anymore.
		mov dword [pagedir + 0], 0
		;  Reload the page structures for the changes to take effect.
		mov ecx, cr3
		mov cr3, ecx
		;  Setup the stack, by moving its VIRTUAL address to esp.
		mov esp, stack_top
		; Push page table virtual address
		push pagetab
		; Push page table physical address
		push pagetab + LINK_ADDR
		;  Pass the PHYSICAL address of the multiboot structure
		;  to the kernel's main() function.
		;  You guessed right, main() has to map it somewhere.
		sub ebx, LINK_ADDR
		push ebx
		;  Finally call the kernel's main() function.
		call kernel_main
	
	;  Halt if main() unexpectedly returns.
	halt:
		cli
		hlt
		jmp halt

	;  Setup the stack whose size is 16 KiB.

section .bss
	align 4096
	global bootstrap_pagetable
	bootstrap_pagetable:
	pagetab:
		resb 4096

	align 4096
	global bootstrap_heap
	bootstrap_heap:
		resb 16384

	align 4096
	; When the heap expands, these will be overwritten!
	global bootstrap_stack
	bootstrap_stack:
	stack_bottom:
		resb 16384
	stack_top:

	align 4096
	;  The page directory.
	global kernel_pagedir
	kernel_pagedir:
	pagedir:
		resb 4096


