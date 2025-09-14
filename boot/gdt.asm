%macro GDT_ENTRY 4
; Arguments:
; 1: base
; 2: limit
; 3: access flags
; 4: flags(4 bits, upper nibble of byte 6)
	%define BASE_LOW (%1 & 0xFFFF)
	%define BASE_MID ((%1 >> 16) & 0xFF)
	%define BASE_HIGH ((%1 >> 24) & 0xFF)
	%define LIMIT_LOW (%2 & 0xFFFF)
	%define LIMIT_HIGH ((%2 >> 16) & 0xF)

	%define FLAGS ((%4 & 0xF) << 4)
	%define ACCESS (%3 & 0xFF)

	dw LIMIT_LOW
	dw BASE_LOW
	db BASE_MID
	db ACCESS
	db (LIMIT_HIGH | FLAGS)
	db BASE_HIGH
%endmacro

gdt_start:
	GDT_ENTRY 0x0, 0x0, 0x0, 0x0 ; null descriptor
	GDT_ENTRY 0x0, 0xFFFFF, 0x9A, 0xC
	GDT_ENTRY 0x0, 0xFFFFF, 0x92, 0xC 
	GDT_ENTRY 0x0, 0x0, 0x9A, 0xA
	GDT_ENTRY 0x0, 0x0, 0x92, 0xC
gdt_end:

gdtr:
    dw gdt_end - gdt_start - 1 ; Limit
    dd gdt_start ; Base
