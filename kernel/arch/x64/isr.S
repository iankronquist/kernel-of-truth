.intel_syntax noprefix

.text

.extern common_interrupt_handler

.global _service_interrupt
.type _service_interrupt, @function
_service_interrupt:
	push rbp
	push rax
	push rbx
	push rcx
	push rdx
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11

	mov ax, ds
	push rax

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	call common_interrupt_handler

	pop rax

	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rdx
	pop rcx
	pop rbx
	pop rax
	pop rbp

	add rsp, 8
	sti
	iret

.macro interrupt number
	.global isr\number
	.type isr\number, @function
	isr\number:
		cli
		push \number
		jmp _service_interrupt
.endm

.macro no_error_code_interrupt number
	.global isr\number
	.type isr\number, @function
	isr\number:
		cli
		push 0
		push \number
		jmp _service_interrupt
.endm

no_error_code_interrupt 0
no_error_code_interrupt 1
no_error_code_interrupt 2
no_error_code_interrupt 3
no_error_code_interrupt 4
no_error_code_interrupt 5
no_error_code_interrupt 6
no_error_code_interrupt 7
interrupt 8
no_error_code_interrupt 9
interrupt 10
interrupt 11
interrupt 12
interrupt 13
interrupt 14
no_error_code_interrupt 15
no_error_code_interrupt 16
interrupt 17
no_error_code_interrupt 18
no_error_code_interrupt 19
no_error_code_interrupt 20
no_error_code_interrupt 21
no_error_code_interrupt 22
no_error_code_interrupt 23
no_error_code_interrupt 24
no_error_code_interrupt 25
no_error_code_interrupt 26
no_error_code_interrupt 27
no_error_code_interrupt 28
no_error_code_interrupt 29
interrupt 30
no_error_code_interrupt 31
no_error_code_interrupt 32
no_error_code_interrupt 33
no_error_code_interrupt 34
