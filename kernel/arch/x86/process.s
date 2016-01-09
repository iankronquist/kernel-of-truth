.intel_syntax noprefix

#struct cpu_state {
#    // pushad
#    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
#    // pushf
#    uint32_t eflags;
#    // mov eax, eip
#    // push eax
#    uint32_t eip;
#    // mov eax, cr3
#    // push eax
#    uint32_t cr3;
#    // mov eax, cs
#    // push eax
#    uint32_t cs;
#};



# Takes the current state of the registers and puts them in *state.
# extern void switch_proc(struct cpu_state *state);
.global switch_proc
switch_proc:

	# Write eip of the destination process to top of the destination process's
	# stack.
	mov eax, [esp]+12
	mov ebx, [esp]+36
	mov [eax], ebx

	# Restore all general purpose registers and stack registers except eax and
	# esp. Use eax as a scratch register for a while.
	mov edi, [esp]+0
	mov esi, [esp]+4
	mov ebp, [esp]+8
	# esp restored later
	mov ebx, [esp]+16
	mov edx, [esp]+20
	mov ecx, [esp]+24
	# eax restored later

	# Restore flags
	mov eax, [esp]+32
	push eax
	popf

	# Restore eip
	mov eax, [esp]+36
	mov eip, eax

	# Restore cr2
	mov eax, [esp]+40
	mov cr3, eax

	mov eax, [esp]+44
	mov cs, eax
	
	# Now that a scratch register is no longer necessary, restore eax
	mov eax, [esp]+28

	# Use the destination process's stack
	mov esp, [esp]+12

	# At the beginning we put the eip at the top of the destination process's
	# stack. Jump to it now.
	jmp [esp]

# Takes the current state of the registers and puts them in *state.
# extern void get_current_regs(struct cpu_state *state);
.global get_current_regs
get_current_regs:
	# Save general purpose registers and stack registers
	mov [esp]+0, edi
	mov [esp]+4, esi
	mov [esp]+8, ebp
	mov [esp]+12, esp
	mov [esp]+16, ebx
	mov [esp]+20, edx
	mov [esp]+24, ecx
	mov [esp]+28, eax

	# Save flags
	pushf
	pop eax
	mov [esp]+32, eax

	# Save eip
	mov eax, eip
	mov [esp]+36, eax

	# Save cr3 (page table)
	mov eax, cr3
	mov [esp]+40, eax

	# Save segment
	mov eax, cs
	mov [esp]+44, eax
	ret

.global set_eip
# Hackish as fuck
set_eip:
	# The return address for this function is at esp+4
	# The return address for the previous function is at esp+8
	# IF THERE WERE NO LOCALS IN THE PARENT FUNCTION AND NO ARGUMENTS AND
	# NOTHING ON THE STACK!!!
	mov eax, [esp+8]
	ret
