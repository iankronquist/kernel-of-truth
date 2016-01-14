section .text
extern syscall_dispatch
extern sys_klog

global _syscall_handler
_syscall_handler:
	pushf
	pushad
	mov ax, ds
	push eax
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov eax, [esp+32]
	.call_klog:
		cmp dword [eax], 0xdeadbeef
		jne .switch_end
		sub    esp, 0xc
		push dword [eax+8]
		call sys_klog
		add    esp, 0x10
	
	.switch_end:
	pop eax

	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	popad
	popf
	iret

global klog
klog:
	push dword 0xdeadbeef
	mov eax, esp
	int 0x80
	pop eax
