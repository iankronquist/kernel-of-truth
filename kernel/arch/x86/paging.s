.intel_syntax noprefix

# extern void flush_tlb(void);
.global flush_tlb
flush_tlb:
	mov	eax,	cr3
	mov	cr3,	eax
	ret

# extern void enable_paging(uint32_t page_dir);
.global enable_paging
enable_paging:
	# Load paging directory into CR3
	mov		eax,	[esp + 4]
	mov		cr3,	eax
	# Enable paging bit in CR0
	mov		eax,	cr0
	or		eax,	0x80000000
	mov		cr0,	eax
	ret

# extern void disable_paging();
.global disable_paging
disable_paging:
	# Disable paging bit in CR0
	mov		eax,	cr0
	and		eax,	~0x80000000
	mov		cr0,	eax
	ret

# extern void just_enable_paging();
.global just_enable_paging
just_enable_paging:
	# Enable paging bit in CR0
	mov		eax,	cr0
	or		eax,	0x80000000
	mov		cr0,	eax
	ret


