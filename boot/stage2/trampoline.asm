[bits 32]

; EAX = page table location
; EBX = boot info location
trampoline_start:
	; Last time using raw addresses, god speed to me 
	;mov ah, '8' ; before pain
	;mov [0xB8000], al

	mov edx, eax

	mov eax, cr4
	bts eax, 5 ; CR4.PAE=1
	mov cr4, eax

	mov cr3, edx

	mov ecx, 0xC0000080 ; IA32_EFER
	rdmsr
	bts eax, 8 ; LME
	wrmsr

	mov eax, cr0
	or eax, 1 << 31
	mov cr0, eax

	jmp 0x18:entry_64

[bits 64]
entry_64:
	; Finally... 64 bit mode
	mov ah, '6'
	mov [0xB8000], ah

	mov rax, 0xFFFFFFFF80000000
  jmp rax 

