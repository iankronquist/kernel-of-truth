.global gdt_flush
.extern gdtp

gdt_flush:
	lgdt gdtp        # Load the GDT with our '_gp' which is a special pointer
	ret

reload_code_segment:
	jmp 0x08+flush2
flush2:
	mov %ax, 0x10      # 0x10 is the offset in the GDT to our data segment
	mov %ds, %ax
	mov %es, %ax
	mov %fs, %ax
	mov %gs, %ax
	mov %ss, %ax
	#jump to an 0x08 offset from flush2.
	ret
