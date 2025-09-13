[bits 32]

;   EAX = phys addr of PML4 (for CR3)
;   EBX = pointer to GDT descriptor
trampoline_start:
	; Last time using raw addresses, god speed to me 
	mov ah, '8' ; before pain
	mov [0xB8000], ah

	jmp $

	
	lgdt [ebx] ; 64 bit GDT
	mov cr3, eax ; Load page tables

	jmp $

	mov eax, cr4
	bts eax, 5 ; CR4.PAE=1
	mov cr4, eax

	jmp $

	mov ecx, 0xC0000080 ; IA32_EFER
	rdmsr
	bts eax, 8 ; LME
	wrmsr

	jmp $

	mov eax, cr0
	or eax, 1 << 31
	mov cr0, eax

	jmp $
	jmp 0x08:entry_64

[bits 64]:
entry_64:
	; Finally... 64 bit mode
	mov ah, '6'
	mov [0xB8000], ah
	hlt

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov rcx, rcx ; make sure rcx carries over
	mov rsp, rcx

	; Jump to higher half here

