ENTRY(_start)

SECTIONS
{
	. = VIRT + 0x100000;
	KERNEL_LOAD = .;
	_int_start_high = ADDR(int_handler) >> 16;
	_kernel_pt1_lonibble = ((_kernel_pt1 - VIRT) >> 8) & 0xf0;
	_kernel_pt1_hiword = ((_kernel_pt1 - VIRT) >> 16) & 0xffff;
	_kernel_pt2_lonibble = ((_kernel_pt2 - VIRT) >> 8) & 0xf0;
	_kernel_pt2_hiword = ((_kernel_pt2 - VIRT) >> 16) & 0xffff;
	_kernel_pd_lonibble = ((kernel_pd - VIRT) >> 8) & 0xf0;
	_kernel_pd_hiword = ((kernel_pd - VIRT) >> 16) & 0xffff;
	_tss_loword = _tss & 0xffff;
	_tss_midbyte = (_tss >> 16) & 0xff;
	_tss_hibyte = (_tss >> 24) & 0xff;
	RX_SECTIONS = .;
	multiboot : AT(0x100000)
	{
		*(multiboot)
	}
	.text ALIGN(4K) : AT(ADDR(.text) - VIRT)
	{
		*(.text)
	}
	int_handler ALIGN(64K) : AT(ADDR(int_handler) - VIRT)
	{
		*(int_handler)
	}
	RX_SECTIONS_LEN = . - RX_SECTIONS;
	RO_SECTIONS = .;
	.rodata ALIGN(4K) : AT(ADDR(.rodata) - VIRT)
	{
		*(.rodata)
	}
	RO_SECTIONS_LEN = . - RO_SECTIONS;
	RW_SECTIONS = .;
	.data ALIGN(4K) : AT(ADDR(.data) - VIRT)
	{
		*(.data)
	}
	.bss ALIGN(4K) : AT(ADDR(.bss) - VIRT)
	{
		*(COMMON)
		*(.bss)
	}
	RW_SECTIONS_LEN = . - RW_SECTIONS;
	KERNEL_LOAD_END = ALIGN(4K);
}

