ENTRY(_start)

SECTIONS {
	. = 1M;
	__kernel_start = .;
	.multiboot BLOCK(4K) : ALIGN(4K) {
		*(.multiboot)
	}

	__text_start = .;
	.text BLOCK(4K) : ALIGN(4K) {
		*(.text)
	}
	__text_end = .;

	__data_start = .;
	.data BLOCK(4K) : ALIGN(4K) {
		*(.data)
	}
	__data_end = .;

	__rodata_start = .;
	.rodata BLOCK(4K) : ALIGN(4K) {
		*(.rodata)
	}
	__rodata_end = .;

	__bss_start = .;
	.bss BLOCK(4K) : ALIGN(4K) {
		*(.bss)
	}
	__bss_end = .;

	__kernel_end = .;
}
