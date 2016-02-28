section .text
%macro syscall0 2
	global %1
	%1:
		mov eax, %2
		int 0x80
		ret
%endmacro

%macro syscall1 2
	global %1
	%1:
		mov ecx, [esp+4]
		mov eax, %2
		int 0x80
		ret
%endmacro

%macro syscall2 2
	global %1
	%1:
		mov ecx, [esp+4]
		mov edx, [esp+8]
		mov eax, %2
		int 0x80
		ret
%endmacro

%macro syscall3 2
	global %1
	%1:
		push ebx
		mov ecx, [esp+4]
		mov edx, [esp+8]
		mov ebx, [esp+12]
		mov eax, %2
		int 0x80
		pop ebx
		ret
%endmacro

global _syscalls_handler
_syscalls_handler:
	pusha
	pushf
	mov ax, ds
	push eax

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	extern syscall_handler
	call syscall_handler

	pop eax

	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	popf
	popa
	iret

syscall1 kputs, 0
