.intel_syntax noprefix

.global disable_A20_interrupts
.type disable_A20_interrupts, @function
disable_A20_interrupts:
       in al, 0x92
       and al, 0xfd
       out 0x92, al
       ret

.global invvpid
.type invvpid, @function
invvpid:
	invvpid rsi, [rdi]
	ret


.global vmclear
.type vmclear, @function
vmclear:
	xor rax, rax
	vmclear [rdi]
	setc al
	setz ah
	ret


.global vmptrld
.type vmptrld, @function
vmptrld:
	xor rax, rax
	vmptrld [rdi]
	setc al
	setz ah
	ret


.global vmptrst
.type vmptrst, @function
vmptrst:
	xor rax, rax
	vmptrst [rdi]
	ret


.global vmread
.type vmread, @function
vmread:
	xor rax, rax
	vmread [rsi], rdi
	setc al
	setz ah
	ret


.global vmwrite
.type vmwrite, @function
vmwrite:
	xor rax, rax
	vmwrite rdi, rsi
	setc al
	setz ah
	ret


.global vmlaunch
.type vmlaunch, @function
vmlaunch:
	xor rax, rax
	vmlaunch
	setc al
	setz ah
	ret

#define Guest_RSP 0x0000681c
#define Guest_RIP 0x0000681e
.global vmlaunch_extended
.type vmlaunch_extended, @function
vmlaunch_extended:
	// Clear return value
	xor rax, rax

	// Fill in guest RSP.
	movq rcx, Guest_RSP
	vmwrite rcx, rsp

	// Fill in guest RIP.
	movq rcx, Guest_RIP
	movq rdx, offset .guest_out
	vmwrite rcx, rdx

	vmlaunch
	// If we got here we're still in the host.
	setc al
	setz ah
	ret

.guest_out:
	// We're in the guest!
	// Set a global indicating that we are in the guest and return 0.
	movb in_vmx_host, 0
	xor rax, rax
	ret



.global vmresume
.type vmresume, @function
vmresume:
	xor rax, rax
	vmresume
	setc al
	setz ah
	ret


.global read_cr0
.type read_cr0, @function
read_cr0:
	mov rax, cr0
	ret


.global write_cr0
.type write_cr0, @function
write_cr0:
	mov cr0, rdi
	ret


# .global read_cr3
# .type read_cr3, @function
# read_cr3:
# 	mov rax, cr3
# 	ret
# 
# 
# .global write_cr3
# .type write_cr3, @function
# write_cr3:
# 	mov cr3, rdi
# 	ret


.global read_cr4
.type read_cr4, @function
read_cr4:
	mov rax, cr4
	ret


.global write_cr4
.type write_cr4, @function
write_cr4:
	mov cr4, rdi
	ret


.global vmxon
.type vmxon, @function
vmxon:
	xor rax, rax
	vmxon [rdi]
	setc al
	setz ah
	ret


.global vmxoff
.type vmxoff, @function
vmxoff:
	xor rax, rax
	vmxoff
	setc al
	setz ah
	ret


.global read_dr7
.type read_dr7, @function
read_dr7:
	mov rax, dr7
	ret


.global read_ss
.type read_ss, @function
read_ss:
	mov ax, ss
	ret


.global read_cs
.type read_cs, @function
read_cs:
	mov ax, cs
	ret


.global read_ds
.type read_ds, @function
read_ds:
	mov ax, ds
	ret


.global read_es
.type read_es, @function
read_es:
	mov ax, es
	ret


.global read_fs
.type read_fs, @function
read_fs:
	mov ax, fs
	ret


.global sgdt
.type sgdt, @function
sgdt:
	sgdt [rdi]
	ret

.global lgdt
.type lgdt, @function
lgdt:
	lgdt [rdi]
	ret


.global sidt
.type sidt, @function
sidt:
	sidt [rdi]
	ret


.global lidt
.type lidt, @function
lidt:
	lidt [rdi]
	ret


.global lsl
.type lsl, @function
lsl:
	lsl ax, si
	ret


.global str
.type str, @function
str:
	str ax
	ret

.global sldt
.type sldt, @function
sldt:
	sldt ax
	ret



.global read_gs
.type read_gs, @function
read_gs:
	mov ax, gs
	ret


# FIXME fix/suppress objtool warnings about frame pointer setup and return
# instruction.
.extern vm_entry_point
.global vm_entry_point_wrapper
.type vm_entry_point_wrapper, @function
vm_entry_point_wrapper:
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rbp
	push rdi
	push rsi
	push rdx
	push rcx
	push rbx
	push rax
	sub rsp, 8

	mov rdi, rax
	mov rsi, rbx
	mov rbp, rsp

	movb in_vmx_host, 1
	call vm_entry_point
	movb in_vmx_host, 0

	add rsp, 8
	pop rax
	pop rbx
	pop rcx
	pop rdx
	pop rsi
	pop rdi
	pop rbp
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15

	vmresume
	# NOT REACHED
	int3


.global read_port
.type read_port, @function
read_port:
	mov rdx, rdi
	in al, dx
	ret

.global write_port
.type write_port, @function
write_port:
	mov rdx, rdi
	mov rax, rsi
	out dx, al
	ret

.global write_port16
.type write_port16, @function
write_port16:
	mov rdx, rdi
	mov rax, rsi
	out dx, ax
	ret

.global read_rsp
.type read_rsp, @function
read_rsp:
	mov rax, rsp
	ret


.global read_rbp
.type read_rbp, @function
read_rbp:
	mov rax, rbp
	ret

.global read_rflags
.type read_rflags, @function
read_rflags:
	pushf
	pop rax
	ret

#.global rdmsr
#.type rdmsr, @function
#rdmsr:
#    movq %rdi, %rcx
#    movq %rdx, %rbx
#    rdmsr
#    movq %rax, (%rsi)
#    movq %rdx, (%rbx)
#    ret

#.global wrmsr
#.type wrmsr, @function
#wrmsr:
#    movq %rdi, %rcx
#    movq %rsi, %rax
#    //movq %rdx, %rdx
#    wrmsr
#    ret



