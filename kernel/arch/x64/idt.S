.intel_syntax noprefix

.text

.global idt_load
.type idt_load, @function
idt_load:
	mov rax, [rsp]
	lidt [rdi]
	mov rax, [rsp]
	ret
