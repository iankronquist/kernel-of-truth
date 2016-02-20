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



; extern void switch_task(uint32_t esp, uint32_t cr3, uint32_t *kernel_esp);
global switch_task
switch_task:
	pusha
	pushf
	mov ax, ds
	push eax
	mov esi, [esp+48]
	mov eax, [esp+52]
	mov [eax], esp
	mov esp, [esp+44]
	mov cr3, esi
	pop eax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	popf
	popa
	sti
	ret

; extern void switch_user_mode_task(uint32_t esp, uint32_t cr3,
;     uint32_t *kernel_esp);
global switch_user_mode_task
switch_user_mode_task:
	pusha
	pushf
	mov ax, ds
	push eax
	mov esi, [esp+48]
	mov eax, [esp+52]
	mov [eax], esp
	mov esp, [esp+44]
	mov cr3, esi
	pop eax
	popf
	popa
	sti

	mov ax, 0x23
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	pop esi
	mov eax, esp
	push 0x23
	push eax
	pushf
	push 0x1b
	call esi
	iret


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
	mov eax, [esp+20]
	call eax
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
