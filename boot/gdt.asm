gdt_start:
	dq 0x0000000000000000 ; Null descriptor
  dq 0x00CF9A000000FFFF ; Code: base=0, limit=4GB, 32-bit
  dq 0x00CF92000000FFFF ; Data: base=0, limit=4GB, read/write

gdtr:
    dw gdt_end - gdt_start - 1 ; Limit
    dd gdt_start ; Base
gdt_end:
