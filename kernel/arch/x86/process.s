section .text

global get_flags
get_flags:
	pushf
	pop eax
	ret


global get_page_dir
get_page_dir:
	mov eax, cr3
	ret

global switch_task
switch_task:
	pusha
	pushf
	mov eax, cr3
	push eax

	mov		eax,	cr0
	and		eax,	~0x80000000
	mov		cr0,	eax


	mov eax, [esp+44]
	mov ebx, [4+eax]
	mov ecx, [8+eax]
	mov edx, [12+eax]
	mov esi, [16+eax]
	mov edi, [20+eax]

	; eax
	mov ebx, [36+esp]
	; eip
	mov ecx, [40+esp]
	; esp
	mov edx, [20+esp]
	; Remove return value
	add edx, 4

	; ebp
	mov esi, [16+esp]
	; Flags
	mov edi, [4+esp]

	mov [eax], ebx
	mov [24+eax], edx
	mov [28+eax], esi
	mov [32+eax], ecx
	mov [36+eax], edi

	; cr3
	pop ebx
	mov [40+eax], ebx

	push ebx

	; Set up the new struct
	mov eax, [48+esp]
	; ebx
	mov ebx, [4+eax]
	; ecx
	mov ecx, [8+eax]
	; edx
	mov edx, [12+eax]
	; esi
	mov esi, [16+eax]
	; edi
	mov edi, [20+eax]
	; ebp
	mov ebp, [28+eax]

	push eax

	; flags
	mov eax, [36+eax]
	push eax

	popf
	pop eax

	; esp
	mov esp, [24+eax]

	; cr3 (page directory)
	push eax
	mov eax, [40+eax]
	mov cr3, eax

	mov		eax,	cr0
	or		eax,	0x80000000
	mov		cr0,	eax

	mov eax, [esp]

	; eip
	mov eax, [32+eax]
	; There are no more registers to use as temporary storage
	xchg eax, [esp]
	; eax
	mov eax, [eax]
	ret

global jump_to_usermode
jump_to_usermode:
	mov ax, 0x23
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov eax, esp
	push 0x23
	push eax
	pushf
	push 0x1b
	extern user_worker
	push user_worker
	iret


global _process_handler
_process_handler:
	push 0
	push 0x20
	pusha
	mov ax, ds
	push eax

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	extern process_handler
	call process_handler

	pop eax

	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	popa
	add esp, 8
	sti
	iret
