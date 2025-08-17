BOOTLOADER_START_ADDR equ 0x7C00
BOOTLOADER_STACK_ADDR equ 0x7BFF

STAGE1_LOAD_ADDR equ 0x100000 ; We will load stage1 at 1MB 
STAGE1_STACK_ADDR equ 0x7E00 ; We will just reuse the 512 bytes from the sector to have a little bit more breathing room


