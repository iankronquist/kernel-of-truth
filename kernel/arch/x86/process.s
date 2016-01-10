.intel_syntax noprefix

.section .text

.global get_flags
get_flags:
	pushf
	pop eax
	ret


.global get_page_dir
get_page_dir:
	mov eax, cr3
	ret

.global switch_task
switch_task:
	pusha
	pushf
	mov eax, cr3
	push eax
	mov eax, [esp+44]
	mov ebx, [4+eax]
	mov ecx, [8+eax]
	mov edx, [12+eax]
	mov esi, [16+eax]
	mov edi, [20+eax]

	# eax
	mov ebx, [36+esp]
	# eip
	mov ecx, [40+esp]
	# esp
	mov edx, [20+esp]
	# Remove return value
	add edx, 4

	# ebp
	mov esi, [16+esp]
	# Flags
	mov edi, [4+esp]

	mov [eax], ebx
	mov [24+eax], edx
	mov [28+eax], esi
	mov [32+eax], ecx
	mov [36+eax], edi

	# cr3
	pop ebx
	mov [40+eax], ebx

	push ebx

	# Set up the new struct
	mov eax, [48+esp]
	# ebx
	mov ebx, [4+eax]
	# ecx
	mov ecx, [8+eax]
	# edx
	mov edx, [12+eax]
	# esi
	mov esi, [16+eax]
	# edi
	mov edi, [20+eax]
	# ebp
	mov ebp, [28+eax]

	push eax

	# flags
	mov eax, [36+eax]
	push eax

	popf
	pop eax

	# esp
	mov esp, [24+eax]

	# cr3 (page directory)
	push eax
	mov eax, [44+eax]
	mov cr3, eax

	mov eax, [esp]

	# eip
	mov eax, [32+eax]
	# There are no more registers to use as temporary storage
	xchg eax, [esp]
	# eax
	mov eax, [eax]
	ret
