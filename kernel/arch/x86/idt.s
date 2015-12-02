.intel_syntax noprefix

.global read_port
read_port:
	mov edx, [esp + 4]
	in al, dx
	ret

.global write_port
write_port:
	mov   edx, [esp + 4]
	mov   al, [esp + 4 + 4]
	out   dx, al
	ret


.global keyboard_handler
keyboard_handler:
	pusha
	mov ax, ds
	push eax

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax


	call    keyboard_irq_handler

	pop eax

	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	popa
	sti
	iret


.global idt_load
idt_load:
	mov eax, [esp+4]
	lidt [eax]
	sti
	ret

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
	push eax

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

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
