OUTPUT_FORMAT("elf64-x86-64","elf64-x86-64","elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
SECTIONS
{
	. = 0x100000;
	_kernel_start = .;

	.text :
	{
		_text_start = .;
		*(.text)
		_text_end = .;
	}

	. = ALIGN(8);
	.data :
	{
		_data_start = .;
		*(.data)
		_data_end = .;
	}
	.rodata : 
	{
		_rodata_start = .;	
		*(.rodata)
		_rodata_end = .;
	}
	.bss :
	{
		_bss_start = .;
		*(.bss)
		_bss_end = .;
	}

	_kernel_end = .;
}