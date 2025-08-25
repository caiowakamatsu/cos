OUTPUT_FORMAT("binary")
ENTRY(kernel_main)

SECTIONS
{
	. = 0xFFFF800000000000;
	.text : { *(.text) }
	.rodata : { *(.rodata) }
	.data : { *(.data) }
	.bss : { *(.bss COMMON) }
}

