section .text

test_main:
	push ebp
	mov ebp, esp
	sub esp, 8
loop:
	sub esp, 12
	push   0x0
	call   kputs
	add esp, 0x10
	jmp loop
	xor eax, eax
	leave
	ret

kputs:
	mov    ecx, [esp+4]
	xor    eax, eax
	int    0x80
	ret
