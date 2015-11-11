.intel_syntax noprefix

.global timer_handler
timer_handler:
	pusha
	mov		ax,		ds
	push	eax

	mov		ax,		0x10
	mov		ds,		ax
	mov		es,		ax
	mov		fs,		ax
	mov		gs,		ax


	call	timer_irq_handler

	pop		eax

	mov		ds,		ax
	mov		es,		ax
	mov		fs,		ax
	mov		gs,		ax

	popa
	sti
	iret
