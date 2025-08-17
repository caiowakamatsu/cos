; stage1.asm
org 0x7C00           ; BIOS loads boot sector here

start:
    cli                 ; disable interrupts
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00      ; simple stack

    ; print string via BIOS teletype
    mov si, msg
print_loop:
    lodsb                ; load byte from DS:SI into AL
    cmp al, 0
    je done
    mov ah, 0x0E         ; teletype print function
    int 0x10
    jmp print_loop
done:

hang:
    hlt
    jmp hang             ; infinite loop

msg db "Hello from Stage 1!",0

times 510-($-$$) db 0     ; fill up to 510 bytes
dw 0xAA55                 ; boot signature
