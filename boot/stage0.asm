org 0x7C00           ; BIOS loads boot sector here

start:
    cli ; Disable ints   
    xor ax, ax ; Setup stack stuff
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7BFF
		sti ; Enable ints

		call enable_a20

hang:
    hlt
    jmp hang             ; infinite loop

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

msg db "Hello from Stage 1!",0

; Footer stuff required for binary bullshittery
times 510-($-$$) db 0
dw 0xAA55
