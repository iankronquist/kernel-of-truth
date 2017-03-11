ENTRY(_start)

SECTIONS {
	/*. = 1M;*/

	. = 0xffff800000000000 + 1M;
	/*. = 0xffffffff80000000 + 0x00200000;*/
	/*. = %KERNEL_BASE% + %KERNEL_LOAD_OFFSET%;*/

	/*.text : AT(%MEMBASE% + %KERNEL_LOAD_OFFSET%) {*/
	/*.text : AT(0 + 0x00200000) {*/
	.text : AT(1M) {
		__kernel_start = .;
		__text_start = .;
		*(.text)
	}

	.data BLOCK(4K) : ALIGN(4K) {
		__text_end = .;
		__data_start = .;
		*(.data)
	}

	.rodata BLOCK(4K) : ALIGN(4K) {
		__data_end = .;
		__rodata_start = .;
		*(.rodata)
	}

	.bss BLOCK(4K) : ALIGN(4K) {
		__bss_start = .;
		__rodata_end = .;
		*(.bss)
	}

	. = ALIGN(4K);
	__bss_end = .;
	__kernel_end = .;
}
