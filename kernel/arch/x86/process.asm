section .text

global get_flags
get_flags:
	pushf
	pop eax
	ret



; extern void switch_task(uint32_t esp, uint32_t cr3, uint32_t *kernel_esp);
global switch_task
switch_task:
	pusha
	pushf
	mov esi, [esp+44]
	mov eax, [esp+48]
	mov [eax], esp
	mov esp, [esp+40]
	mov cr3, esi
	; FIXME store & restore segment selectors
	popf
	popa
	sti
	ret



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
