#.intel_syntax noprefix

#define MB_MAGIC    0x1badb002
#define MB_ALIGN    (1 << 0)
#define MB_INFO     (1 << 1)
#define MB_FLAGS    (MB_ALIGN | MB_INFO)
#define MB_CHECKSUM -(MB_MAGIC + MB_FLAGS)

#define KB 0x400
#define MB 0x100000
#define PL1_SIZE 0x1000


.section .multiboot,"aw",@progbits

	.align 4
	.long MB_MAGIC
	.long MB_FLAGS
	.long MB_CHECKSUM

.global _start
_start:
.extern kernel_main
	call kernel_main
	# We should never reach here.
	cli
	hlt

.section .bss,"aw",@nobits

# init_stack is 16 kb
init_stack:
#	.skip (16 * KB)

init_pl1:
#	.skip PL1_SIZE
