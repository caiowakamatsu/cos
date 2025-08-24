BOOTLOADER_START_ADDR equ 0x7C00
BOOTLOADER_STACK_ADDR equ 0x7BFF

STAGE1_LOAD_ADDR equ 0x8000 
STAGE1_STACK_ADDR equ 0x7FFF

; dl = Drive number 
; Stack should be 
; 2 : Number of sectors to read (127 max)
; 2 : Segment
; 2 : Offset
; 2 : Starting sector # low 
; 2 : Starting sector # high (2^32 max)
; THIS FUNCTION DOES NOT DO ERROR CHECKING, GOOD LUCK
read_lba:
	; Store read data in buffer
	; Numbers of sectors to read
	pop bx

    pop ax  ; Number of sectors
    mov [common__lba_packet + 0], ax
    
    pop ax  ; Segment
    mov [common__lba_packet + 2], ax
    
    pop ax  ; Offset
    mov [common__lba_packet + 4], ax
    
    pop ax  ; Starting sector # low
    mov [common__lba_packet + 6], ax
    
    pop ax  ; Starting sector # high  
    mov [common__lba_packet + 8], ax

	; Align stack for data packet write
	mov cx, sp
	and cx, 3 ; cx = sp % 4 
	jz read_lba__aligned
	sub sp, cx
read_lba__aligned:
	; Write actual packet onto the stack (but backwards)
	
	; Upper 16 bits (but stored at 4 byte) of LBA sector #
	xor ax, ax
	push ax
	push ax

	; Lower 32 bits of LBA sector #
	mov ax, [common__lba_packet + 8] ; Is this backwards?
	push ax
	mov ax, [common__lba_packet + 6]
	push ax

	; Offset to write to
	mov ax, [common__lba_packet + 4]
	push ax

	; Segment to write to 
	mov ax, [common__lba_packet + 2]
	push ax

	; Sector count
	mov ax, [common__lba_packet + 0]
	push ax

	; Reserved must be 0 + Size (we cant push 1 byte at a time)
	mov ax, 0x1000
	push ax

	; Stack grows backwards, so the start of the packet is after we've written everything
	mov ax, ss
	mov ds, ax
	mov si, sp

	mov ah, 0x42
	int 0x13

	add sp, 16 ; Data packet 
	add sp, cx ; Restore the stack after the bullshittery we did

	push bx

	ret

; si = str1
; di = str2
; returns:
;		bl=1 if equal
;		bl=0 otherwise
strcmp:
	push ax
.loop:
	mov al, [si]
	cmp al, [di]
	jne .different ; Are the chars different?
	cmp al, 0
	je .equal
	inc si
	inc di
	jmp .loop
.equal:
	pop ax
	mov bl, 1
	ret
.different:
	pop ax
	mov bl, 3
  ret

; Load filesystem metadata
; Returns entry count in ECX
load_fs_metadata:
	mov ax, 0
	push ax
	mov ax, 1
	push ax

	mov ax, 0
	push ax
	mov ax, 0x1000
	push ax

	mov ax, 1
	push ax

	call read_lba
	jc load_fs_metadata__end
	mov ecx, [0x1000 + 0] ; Location of entry count

load_fs_metadata__end:
	ret

; Load entry metadata with name check
; EAX = Entry index
load_entry_metadata:
	push eax ; be nice and dont globber 
	push eax ; push we need to store
	xor cx, cx
	push cx
	pop eax
	add eax, 2
	push ax ; Write the actual value here

	mov ax, 0
	push ax
	mov ax, 0x1000
	push ax

	mov ax, 1
	push ax

	call read_lba
	pop eax
	ret

; Expects identifier name in SI
; Loads temporary data at 0x1000
; Returns sector count ECX, lba start in EBX
; Sets carry if error
find_entry:
	push si
	; Load big filesystem metadata
	call load_fs_metadata
	jc .end

	xor eax, eax
	; Loop through entries until we find the one that matches the string
.loop:
	call load_entry_metadata
	jc .end ; Error?
	; Compare the strings
	mov di, 0x1000
	pop si
	call strcmp
	cmp bl, 1
	je .end

	; Didn't find it, try again
	inc eax
	jmp .loop

.end_not_found:
	stc
	ret 
.end:

	; Load parameters
	mov ebx, [0x1000 + 32] ; Sector start
	mov ecx, [0x1000 + 36] ; Sector count

	ret


; This is not the actual packet, because of the shit I'm doing with the stack
; We need somewhere to put stuff "in the meantime", so everything is stored here
common__lba_packet:
	dw 0 ; (0) Number of sectors to read
	dw 0 ; (2) Segment
	dw 0 ; (4) Offset
	dd 0 ; (6) Lower 32 bits for LBA start sector
