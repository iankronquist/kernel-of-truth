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
	add esp, 49
	sti
	jmp ebx


# void _resume_proc2(struct regs state);
.global _resume_proc
_resume_proc2:
	cli
	# ebx gets new esp
	mov ebx, [esp + 4]
	# ecx gets new ebp
	mov ecx, [esp + 8]
	# edx gets new eip
	mov edx, [esp + 12]
	mov ebp, ecx
	mov esp, edx

	pop eax

	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	popa

	sti
	iret
	#jmp eip



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
	# Remove things pushed on stack by interrupt handler
	add esp, 8
	sti
	# Jump to the location of the instruction pointer saved to the stack
	jmp [esp-8]
	#iret
