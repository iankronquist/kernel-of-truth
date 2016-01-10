.intel_syntax noprefix

# extern void enter_proc(uint32_t esp)
.global enter_proc
enter_proc:
	push ebp
	mov ebp, esp

	# Extra space for eip
	push eax
	# 1 * 4 = 4
	pushf
	# 7 * 4 = 28
	pushad
	
	# Save esp
	# 1 * 4 = 4
	mov eax, esp
	push eax
	# Pushed a total of 28 + 4 + 4 + 4 = 40 bytes

	# Put the return value to resume at the space allocated at the beginning
	mov eax, [ebp+4]
	mov [esp+40], eax

	# Now for the grand switch!
	mov esp, [ebp+8]
	popad
	popf
	ret

# extern void set_up_stack(uint32_t new_stack, uint32_t new_eip)
.global set_up_stack
set_up_stack:
	push ebp
	mov ebp, esp

	# eax holds new esp
	mov eax, [ebp+8]

	# ebx holds new eip
	mov ebx, [ebp+12]

	# Give the new process the flags of the current process
	# ecx holds the flags
	pushf
	pop ecx

	# eip
	mov [eax+0], ebx

	# flags
	mov [eax+4], ecx

	# EDI, ESI, EBP, EBX, EDX, ECX, and EAX.
	# edi
	mov dword ptr [eax+8], 0
	# esi
	mov dword ptr [eax+12], 0
	# ebp
	mov dword ptr [eax+16], eax
	# ebx
	mov dword ptr [eax+20], 0
	# edx
	mov  dword ptr [eax+24], 0
	# ecx
	mov dword ptr [eax+28], 0
	# eax
	mov dword ptr [eax+32], 0

	# esp
	mov [eax+36], eax


	mov esp, ebp
	pop ebp
	ret
