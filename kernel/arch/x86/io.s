global read_port
read_port:
	mov edx, [esp + 4]
	in al, dx
	ret

global write_port
write_port:
	mov   edx, [esp + 4]
	mov   al, [esp + 4 + 4]
	out   dx, al
	ret

global read_port16
read_port16:
	mov edx, [esp + 4]
	in ax, dx
	ret

global write_port16
write_port16:
	mov   edx, [esp + 4]
	mov   ax, [esp + 4 + 4]
	out   dx, ax
	ret

global read_port32
read_port32:
	mov edx, [esp + 4]
	in eax, dx
	ret

global write_port32
write_port32:
	mov   edx, [esp + 4]
	mov   eax, [esp + 4 + 4]
	out   dx, eax
	ret
