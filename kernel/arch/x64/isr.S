.text

#define PIT_Interrupt_Number 0x20

.extern interrupts_dispatcher
.global _service_interrupt
.type _service_interrupt, @function
_service_interrupt:
	# Save all registers.
	push %rax
	push %rbx
	push %rcx
	push %rdx
	push %rbp
	push %rsi
	push %rdi
	push %r8
	push %r9
	push %r10
	push %r11
	push %r12
	push %r13
	push %r14
	push %r15

	# Makes logging nice, not really necessary.
	xor %rax, %rax
	# Save data segment.
	mov %ds, %ax
	push %rax

	# Load kernel data segment.
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	# If this isn't a timer interrupt, feed the entropy pool with the last
	# byte of the tick counter.
	testq $PIT_Interrupt_Number, 16*8(%rsp)
	jne .dispatch
	rdtsc
	mov %di, %ax
	.extern entropy_pool_seed
	call entropy_pool_seed

	# Call interrupt dispatcher.
.dispatch:
	.extern interrupts_dispatcher
	call interrupts_dispatcher

	# Restore data segment.
	pop %rax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	# Restore all registers.
	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rdi
	pop %rsi
	pop %rbp
	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax

	# Add back extra bit pushed to the stack by the interrupt handler.
	add $16, %rsp
	iretq


# Certain interrupts push an error code to the stack.
# When one of these interrupts happens, push the interrupt number after
# the error code which is already on the stack.
.macro interrupt number
	.global isr\number
	.type isr\number, @function
	isr\number:
		cli
		push $\number
		jmp _service_interrupt
.endm

# Other interrupts don't push an error code to the stack. In the name of
# consistency, push a dummy value, and then the interrupt number
.macro no_error_code_interrupt number
	.global isr\number
	.type isr\number, @function
	isr\number:
		cli
		push $0
		push $\number
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
no_error_code_interrupt 35
no_error_code_interrupt 36
no_error_code_interrupt 37
no_error_code_interrupt 38
no_error_code_interrupt 39
no_error_code_interrupt 40
no_error_code_interrupt 41
no_error_code_interrupt 42
no_error_code_interrupt 43
no_error_code_interrupt 44
no_error_code_interrupt 45
no_error_code_interrupt 46
no_error_code_interrupt 47
