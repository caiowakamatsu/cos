; File for reading memory memorys using e820

; We store the amount of entries at 0x1000, buffer starts at 0x1010
load_e820:
	push ax
	push bx
	push cx
	push dx
	push si
	push sp
	push bp

	xor ax, ax
	; Location for the entry count
	mov [0x1200], ax
	; 0x0000:0x1002 for buffer location
	mov es, ax
	mov ax, 0x1210
	mov di, ax

	xor ebx, ebx ; Clear EBX
	mov edx, 0x534D4150
.loop:
	mov ecx, 0x18
	mov eax, 0xE820
	int 0x15
	jc .end

	; Increment element count by 1
	mov ax, [0x1200]
	inc ax
	mov [0x1200], ax

	; Point to the next element
	add di, 0x18

	cmp ebx, 0
	je .end

	; Loop again
	jmp .loop

.end:

	pop bp
	pop sp
	pop si
	pop dx
	pop cx
	pop bx
	pop ax

	ret
