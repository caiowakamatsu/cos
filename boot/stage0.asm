bits 16
org 0x7C00           ; BIOS loads boot sector here

start:

    xor ax, ax ; Setup stack stuff
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7BFF
		sti ; Enable ints

		call enable_a20
		call load_stage1

		jmp 0x0:0x800

print_halted:
    mov si, halted_msg
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

stage1_identifier db "stage1.bin", 0

halted_msg:
	dq "HALTED" 0

hang:
		call print_halted
    hlt
    jmp hang             ; infinite loop

%include "boot/common.asm"

load_stage1:
	mov si, stage1_identifier
	call find_entry
	jc hang
	; dl is already loaded by BIOS (thank you)

	; Load starting from sector 1
	mov ax, 0 ; High bit 0
	push ax
	mov ax, bx ; Low bit 1
	add ax, 1
	mov [0x1000], ax
	push ax

	; Segment
	mov ax, 0
	push ax

	; Offset is 0 
	mov ax, 0x800
	push ax

	; Number of sectors to read
	mov ax, cx
	mov [0x1010], ax
	push ax

	call read_lba
	jc hang

load_stage1__exit:
	ret

; Tries to enable the A20 line
enable_a20:
	call check_a20 ; Do we need to enable the A20 line?
	cmp ax, 0 
	je enable_a20__exit ; No, it's already enabled, keep going :) 
	mov ax, 0x2401 ; Yes we need to enable it ourselves
	int 0x15
	jc hang ; failed to enable :(

enable_a20__exit:
	ret

; Does it even need to be enabled?
; ax=0 disabled, ax=1 enabled
check_a20:
	pushf
  push ds
	push es
	push di
	push si

	cli

	xor ax, ax ; ax = 0
	mov es, ax

	not ax ; ax = 0xFFFF
	mov ds, ax

	mov di, 0x0500
	mov si, 0x0510

	mov al, byte [es:di]
	push ax

	mov al, byte [ds:si]
	push ax

	mov byte [es:di], 0x00
	mov byte [ds:si], 0xFF

	cmp byte [es:di], 0xFF

	pop ax
	mov byte [ds:si], al

	pop ax
	mov byte [es:di], al

	mov ax, 0
	je check_a20__exit

	mov ax, 1

check_a20__exit:
	pop si
	pop di
	pop es
	pop ds
	popf

	ret

msg db "stage 0 - end",0

; Footer stuff required for binary bullshittery
times 510-($-$$) db 0
dw 0xAA55
