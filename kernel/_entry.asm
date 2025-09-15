[BITS 64]

k_entry:
	mov ah, '7'
	mov [0xB8000], ah
	and rsp, ~0xF           ; Align stack to 16 bytes
  sub rsp, 8              ; Account for call pushing 8 bytes (total 16-byte aligned)

	mov edi, ebx

	jmp k_main

	;call k_main
.hang:
	hlt
	jmp .hang

global k_entry
extern k_main 

