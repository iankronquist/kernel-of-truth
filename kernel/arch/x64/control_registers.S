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

.global read_cr2
.type read_cr2, @function
read_cr2:
	mov %cr2, %rax
	ret
