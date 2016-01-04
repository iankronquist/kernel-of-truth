.intel_syntax noprefix

.global keyboard_handler
keyboard_handler:
	pusha
	mov ax, ds
	push eax

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax


	call    keyboard_irq_handler

	pop eax

	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	popa
	sti
	iret


