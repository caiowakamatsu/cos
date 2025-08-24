OUTPUT_FORMAT("binary")
ENTRY(stage2_main)

SECTIONS
{
    . = 0x8000;
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss COMMON) }
}

