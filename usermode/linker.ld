ENTRY("_start")

SECTIONS
{
	. = 0x10000;

	.text ALIGN(4K) :
	{
		crt0.o(.text)
		*(.init)
		*(.fini)
		*(.text)
	}

	.rodata ALIGN(4K) :
	{
		*(.rodata)
	}

	.data ALIGN(4K) :
	{
		*(.data)
	}

	.bss ALIGN(4) :
	{
		*(.bss)
		*(COMMON)
	}
}

