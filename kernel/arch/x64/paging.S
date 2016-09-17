.text

.global write_cr3
.type write_cr3, @function
write_cr3:
	mov %rdi, %cr3
	ret

.global read_cr3
.type read_cr3, @function
read_cr3:
	mov %cr3, %rax
	ret
