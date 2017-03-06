ENTRY(_start)

SECTIONS {
	. = 1M;
	__kernel_start = .;
	.multiboot BLOCK(4K) : ALIGN(4K) {
		*(.multiboot)
	}

	.text BLOCK(4K) : ALIGN(4K) {
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
