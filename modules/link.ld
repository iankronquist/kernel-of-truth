SECTIONS {

	.text BLOCK(4K) : ALIGN(4K) {
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
