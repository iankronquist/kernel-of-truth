.intel_syntax noprefix

.global idt_load
idt_load:
	#loads the IDT table. idtp is the IDT pointer.
	mov eax, [esp+4]
	lidt [eax]
	ret

.extern generic_interrupt
.global common_handler

.macro interrupt number
	.global isr\number
	isr\number:
		cli
		push \number
		jmp common_handler
.endm

.macro no_error_code_interrupt number
	.global isr\number
	isr\number:
		cli
		push 0
		push \number
		jmp common_handler
.endm


common_handler:
	pusha
	mov ax, ds

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	push eax

	call common_interrupt_handler
	pop eax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	popa
	add esp, 8
	sti
	iret

loop:
	jmp loop

.global interrupt_handler_any
interrupt_handler_any:
	push 7
	jmp common_handler

.global no_error_code_interrupt_handler_any
no_error_code_interrupt_handler_any:
	push 0
	push 8
	jmp common_handler


no_error_code_interrupt 0
no_error_code_interrupt 1
no_error_code_interrupt 2
no_error_code_interrupt 3
no_error_code_interrupt 4
no_error_code_interrupt 5
no_error_code_interrupt 6

no_error_code_interrupt 7
no_error_code_interrupt 8
no_error_code_interrupt 9
no_error_code_interrupt 10
no_error_code_interrupt 11
no_error_code_interrupt 12
no_error_code_interrupt 13
no_error_code_interrupt 14
no_error_code_interrupt 15
no_error_code_interrupt 16
no_error_code_interrupt 17
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
no_error_code_interrupt 30
no_error_code_interrupt 31
