[BITS 64]

k_entry:
	mov ah, '7'
	mov [0xB8000], ah

	mov rax, 0xFFFFFE8000000000
	or rax, 16 * 4096
	dec rax
	mov rsp, rax

	and rsp, ~0xF
	mov edi, ebx

	call k_main
.hang:
	hlt
	jmp .hang

global k_entry
extern k_main 

