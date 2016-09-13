ENTRY(_start)

SECTIONS
{
	/* Begin putting sections at 1 MiB */
	. = 1M;
	kernel_start = .;

	/*
	.multiboot BLOCK(4K) : ALIGN(4K)
	{
		__bootstrap_start = .;
		__bootstrap_end = .;
	}
		/* *bootstrap_paging.x86.o(.text)*/

	/* Text segment */
	.text BLOCK(4K) : ALIGN(4K)
	{
		__text_start = .;
		*(.multiboot)
		*(.text)
		__text_end = .;
	}

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		__rodata_start = .;
		*(.rodata)
		__rodata_end = .;
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		__data_start = .;
		*(.data)
		__data_end = .;
	}

	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		__bss_start = .;
		*(COMMON)
		*(.bss)
		__bss_end = .;
	}
	kernel_end = .;

}
