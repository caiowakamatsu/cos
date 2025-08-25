org 0x800
bits 16

stage1_entry:
	; This is where VBE will be loaded
	; This is where reserved memory regions will be loaded
	call load_e820

	; Load 32 bit mode code
	call load_stage2
	; No more interrupts
	cli

	; Load GDT
	lgdt [gdtr]
	; Enable protected mode CR0
	mov eax, cr0
	or eax, 1 ; enable protected mode 
	mov cr0, eax
	jmp 0x08:0x8000
	; Off to protected mode we goooooo

%include "boot/memory.asm"
%include "boot/common.asm"
%include "boot/gdt.asm"


load_stage2:
	; Load sector2 data from filesystem
	mov si, stage2_identifier
	call find_entry
	jc hang

	; Sector starts at STAGE2_SECTOR_START (which is... hopefully under 2^16)
	mov ax, 0
	push ax
	mov ax, bx
	add ax, 1
	push ax

	; Segment
	mov ax, 0
	push ax

	; Offset
	mov ax, 0x8000
	push ax

	; Number of sectors to read
	mov ax, cx
	push ax

	call read_lba
	jc hang
	ret

hang:
	call print_halted
	hlt
	jmp hang

print_halted:
    mov si, msg 
.print_loop:
    lodsb               ; Load byte from DS:SI into AL, increment SI
    cmp al, 0
    je .done
    mov ah, 0x0E        ; BIOS teletype function
    mov bh, 0           ; page 0
    mov bl, 0x07        ; normal attribute
    int 0x10
    jmp .print_loop
.done:
    ret

;---------------------------------
; Data section
stage2_identifier db "stage2.bin", 0
msg db "stage 1 - end", 0


