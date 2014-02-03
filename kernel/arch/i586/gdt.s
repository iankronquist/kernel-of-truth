.global gdt_flush
.extern gdtp

gdt_flush:
	lgdt gdtp        # Load the GDT with our '_gp' which is a special pointer
	mov %ax, 0x10      # 0x10 is the offset in the GDT to our data segment
	mov %ds, %ax
	mov %es, %ax
	mov %fs, %ax
	mov %gs, %ax
	mov %ss, %ax
	jmp flush2   
flush2:
	ret
