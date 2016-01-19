section .text
extern syscall_dispatch
extern sys_klog
extern sys_kputs
extern sys_spawn
extern sys_exit
extern sys_exec
extern bad_syscall

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
		jne .call_kputs
		sub    esp, 0xc
		push dword [eax+8]
		call sys_klog
		add    esp, 0x10
		jmp .switch_end
	.call_kputs:
		cmp dword [eax], 0x8badf00d
		jne .call_spawn
		sub    esp, 0xc
		push dword [eax+8]
		call sys_kputs
		add    esp, 0x10
		jmp .switch_end
	.call_spawn:
		cmp dword [eax], 0x00bada55
		jne .call_exec
		sub    esp, 0xc
		push dword [eax+8]
		call sys_spawn
		add    esp, 0x10
		jmp .switch_end
	.call_exec:
		cmp dword [eax], 0xd15ea5ed
		jne .call_exit
		sub    esp, 0xc
		push dword [eax+8]
		call sys_exec
		add    esp, 0x10
		jmp .switch_end
	.call_exit:
		cmp dword [eax], 0x0defaced
		jne .call_bad_syscall
		;sub    esp, 0xc
		;push dword [eax+8]
		; Takes no arguments
		call sys_exit
		;add    esp, 0x10
		jmp .switch_end
	; This is useful for debugging
	.call_bad_syscall
		push dword [eax]
		call bad_syscall
		add esp, 4
		; fall through to the end of the switch statement


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
	ret

global kputs
kputs:
	push dword 0x8badf00d
	mov eax, esp
	int 0x80
	pop eax
	ret

global spawn
spawn:
	push dword 0x00bada55
	mov eax, esp
	int 0x80
	pop eax
	ret

global exec
exec:
	push dword 0xd15ea5ed
	mov eax, esp
	int 0x80
	pop eax
	ret

global exit
exit:
	push dword 0x0defaced
	mov eax, esp
	int 0x80
	; NOT REACHED
	.hang:
	jmp .hang
	pop eax
