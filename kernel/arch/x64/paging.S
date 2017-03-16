.text

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
