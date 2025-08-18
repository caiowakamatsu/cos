org 0x8000
bits 16

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

halted_msg:
	db "HALTED" 0

;---------------------------------
; Data section
msg db "Hi from stage 1", 0


times 512-($-$$) db 0
