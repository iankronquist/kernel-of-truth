.global isr0
.global isr1
.global isr2
.global isr3
.global isr4
.global isr5
.global isr6
.global isr7
.global isr8
.global isr9
.global isr10
.global isr11
.global isr12
.global isr13
.global isr14
.global isr15
.global isr16
.global isr17
.global isr18
.global isr19
.global isr20
.global isr21
.global isr22
.global isr23
.global isr24
.global isr25
.global isr26
.global isr27
.global isr28
.global isr29
.global isr30
.global isr31


#Division By Zero Exception
isr0:
	cli
	push 0x0
	push 0x0
	jmp isr_common_stub
#Debug Exception
isr1:
	cli
	push 0x0
	push 0x0
	jmp isr_common_stub
#Non Maskable Interrupt Exception
isr2:
	cli
	push 0x0
	push 0x0
	jmp isr_common_stub
#Breakpoint Exception
isr3:
	cli
	push 0x0
	push 0x0
	jmp isr_common_stub
#Into Detected Overflow Exception
isr4:
	cli
	push 0x0
	push 0x0
	jmp isr_common_stub
#Out of Bounds Exception
isr5:
	cli
	push 0x0
	push 0x0
	jmp isr_common_stub
#Invalid Opcode Exception
isr6:
	cli
	push 0x0
	push 0x0
	jmp isr_common_stub
#No Coprocessor Exception
isr7:
	cli
	push 0x0
	push 0x0
	jmp isr_common_stub
#Double Fault Exception
isr8:
	cli
	push 0x0
	jmp isr_common_stub
#Coprocessor Segment Overrun Exception
isr9:
	cli
	push 0x0
	jmp isr_common_stub
#Bad TSS Exception
isr10:
	cli
	push 0x0
	jmp isr_common_stub
#Segment Not Present Exception
isr11:
	cli
	push 0x0
	jmp isr_common_stub
#Stack Fault Exception
isr12:
	cli
	push 0x0
	jmp isr_common_stub
#General Protection Fault Exception
isr13:
	cli
	push 0x0
	jmp isr_common_stub
#Page Fault Exception
isr14:
	cli
	push 0x0
	jmp isr_common_stub
#Unknown Interrupt Exception
isr15:
	cli
	push 0x0
	jmp isr_common_stub
#Coprocessor Fault Exception
isr16:
	cli
	push 0x0
	jmp isr_common_stub
#Alignment Check Exception (486+)
isr17:
	cli
	push 0x0
	jmp isr_common_stub
#Machine Check Exception (Pentium/586+)
isr18:
	cli
	push 0x0
	jmp isr_common_stub
#Reserved Exceptions
isr19:
	cli
	push 0x0
	jmp isr_common_stub
#Reserved Exceptions
isr20:
	cli
	push 0x0
	jmp isr_common_stub
#Reserved Exceptions
isr21:
	cli
	push 0x0
	jmp isr_common_stub
#Reserved Exceptions
isr22:
	cli
	push 0x0
	jmp isr_common_stub
#Reserved Exceptions
isr23:
	cli
	push 0x0
	jmp isr_common_stub
#Reserved Exceptions
isr24:
	cli
	push 0x0
	jmp isr_common_stub
#Reserved Exceptions
isr25:
	cli
	push 0x0
	jmp isr_common_stub
#Reserved Exceptions
isr26:
	cli
	push 0x0
	jmp isr_common_stub
#Reserved Exceptions
isr27:
	cli
	push 0x0
	jmp isr_common_stub
#Reserved Exceptions
isr28:
	cli
	push 0x0
	jmp isr_common_stub
#Reserved Exceptions
isr29:
	cli
	push 0x0
	jmp isr_common_stub
#Reserved Exceptions
isr30:
	cli
	push 0x0
	jmp isr_common_stub
#Reserved Exceptions
isr31:
	cli
	push 0x0
	jmp isr_common_stub


.extern fault_handler

isr_common_stub:
    pusha
    push ds
    push es
    push fs
    push %gs
    mov %ax, 0x10   
    mov %ds, %ax
    mov %es, %ax
    mov %fs, %ax
    mov %gs, %ax
    mov %eax, %esp   
    push eax
    mov %eax, fault_handler
    call eax       
    pop eax
    pop %gs
    pop %fs
    pop es
    pop ds
    popa
    add %esp, 8     
    iret           

