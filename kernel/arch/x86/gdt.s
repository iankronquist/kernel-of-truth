section .text
extern gdtp

global gdt_flush
gdt_flush:
	; Load the GDT with our gdtp which is a structure which contains a pointer
	; to the gdt
	lgdt [gdtp]
	jmp reload_code_segment
	ret

global reload_code_segment
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

global tss_flush
tss_flush:
   mov ax, 0x28
   ltr ax
   ret
