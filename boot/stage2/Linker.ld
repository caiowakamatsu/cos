OUTPUT_FORMAT("binary")
ENTRY(stage2_main)

SECTIONS
{
    . = 0x0;              /* load at offset 0 in binary */
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss COMMON) }
}

