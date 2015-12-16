.intel_syntax noprefix

# void resume_proc(uint32_t new_eip, uint32_t new_ebp, uint32_t new_esp)
.global _resume_proc
_resume_proc:
	cli
	# ebx gets new esp
	mov ebx, [esp + 4]
	# ecx gets new ebp
	mov ecx, [esp + 8]
	# edx gets new eip
	mov edx, [esp + 12]
	mov ebp, ecx
	mov esp, edx
	sti
	jmp ebx

.global process_handler
process_handler:
	cli
	push 0
	push 0x20
	pusha
	mov		ax,		ds
	push	eax

	mov		ax,		0x10
	mov		ds,		ax
	mov		es,		ax
	mov		fs,		ax
	mov		gs,		ax


	call	scheduler_wakeup

	pop		eax

	mov		ds,		ax
	mov		es,		ax
	mov		fs,		ax
	mov		gs,		ax

	popa
	# Remove error code and interrupt number from the stack
	add esp, 8
	sti
	iret
