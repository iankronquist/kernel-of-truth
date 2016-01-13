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
