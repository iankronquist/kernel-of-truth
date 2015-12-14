.intel_syntax noprefix

# void resume_proc(uint32_t new_eip, uint32_t new_ebp, uint32_t new_esp)
.global _resume_proc
_resume_proc:
	cli
	# ebx gets new eip
	mov ebx, [esp + 12]
	# ecx gets new ebp
	mov ecx, [esp + 8]
	# edx gets new esp
	mov edx, [esp + 4]
	mov ebp, ecx
	mov esp, edx
	sti
	jmp ebx
