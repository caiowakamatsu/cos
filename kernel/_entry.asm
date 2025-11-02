[BITS 64]

k_entry:
	mov ah, '7'
	mov [0xB8000], ah

mov rsp, 0xFFFFFFFF7FFF4000
and rsp, ~0xF
	mov edi, ebx
	call k_main
.hang:
	hlt
	jmp .hang

global k_entry
extern k_main 

