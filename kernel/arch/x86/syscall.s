section .text
extern syscall_dispatch
extern sys_klog

global _syscall_handler
_syscall_handler:

	;mov ax, ds
	;push eax
	;mov ax, 0x10
	;mov ds, ax
	;mov es, ax
	;mov fs, ax
	;mov gs, ax

	.call_klog:
		cmp dword [eax], 0xdeadbeef
		jne .switch_end
		sub    esp, 0xc
		push dword [eax+8]
		call sys_klog
		add    esp, 0x10
	
	.switch_end:
	;pop eax

	;mov ds, ax
	;mov es, ax
	;mov fs, ax
	;mov gs, ax

	sti
	iret

global klog
klog:
	push dword 0xdeadbeef
	mov eax, esp
	int 0x80
	pop eax
