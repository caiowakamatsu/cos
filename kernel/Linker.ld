OUTPUT_FORMAT("binary")

SECTIONS
{
	. = 0xFFFFFF0000000000;
	.text : { *(.text) }
	.rodata : { *(.rodata) }
	.data : { *(.data) }
	.bss : { *(.bss COMMON) }
}

