[BITS 32]

stage2_entry:
  mov ax, 0x10      
  mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov esp, 0x1FFFFF
	call stage2_main        
.hang:
	jmp .hang

global stage2_entry
extern stage2_main

