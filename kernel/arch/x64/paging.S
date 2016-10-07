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

.global invalidate_page
.type invalidate_page, @function
invalidate_page:
	push %rdi
	invlpg (%esp)
	pop %rax
	ret

.global invalidate_tlb
.type invalidate_tlb, @function
invalidate_tlb:
	mov %cr3, %rax
	mov %rax, %cr3
	ret
