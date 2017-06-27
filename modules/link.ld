SECTIONS {

	. = ALIGN(4K);

	.text : {
		__module_start = .;
		__module_rx_start = .;
		*(.text*)
		*(.plt*)
	}

	.init_array : {
		*(.init_array*)
		*(.ctors*)
	}

	.fini_array : {
		*(.init_array*)
		*(.dtors*)
	}

	. = ALIGN(4K);
    __module_rx_end = .;
    __module_rw_start = .;

	.data : {
		*(.data*)
	}

	. = ALIGN(4K);
    __module_rw_end = .;
    __module_ro_start = .;

	.rodata : {
		*(.rodata*)
		*(.got*)
	}

	. = ALIGN(4K);
    __module_ro_end = .;
    __module_bss_start = .;

	.bss : {
		*(.bss*)
	}

	. = ALIGN(4K);
    __module_bss_end = .;
    __module_end = .;
}
