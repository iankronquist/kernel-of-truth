ENTRY(_start)

SECTIONS {

	. = 1M;
	.text : AT(1M) {
		*(.text)
	}

	.data BLOCK(4K) : ALIGN(4K) {
		*(.data)
	}

	.rodata BLOCK(4K) : ALIGN(4K) {
		*(.rodata)
	}

	.bss BLOCK(4K) : ALIGN(4K) {
		*(.bss)
	}

	. = ALIGN(4K);
}
