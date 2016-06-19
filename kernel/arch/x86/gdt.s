section .text

global gdt_flush
gdt_flush:
	; Load the GDT with our gdtp which is a structure which contains a pointer
	; to the gdt
	lgdt [esp+4]
	call reload_code_segment
	ret

reload_code_segment:
	jmp 0x08:flush2
flush2:
	; 0x10 is the offset in the GDT to our data segment
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	ret
