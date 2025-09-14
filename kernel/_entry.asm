[BITS 64]

k_entry:
	mov ah, '7'
	mov [0xB8000], ah
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov rsp, 0xFFFFFFFF7FFF0000 + (16 * 1024)
	and rsp, ~0xF           ; Align stack to 16 bytes
  sub rsp, 8              ; Account for call pushing 8 bytes (total 16-byte aligned)
	jmp k_main

	;call k_main
.hang:
	hlt
	jmp .hang

global k_entry
extern k_main 

