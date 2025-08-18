BUILD_DIR := build
STAGE0 := $(BUILD_DIR)/stage0.bin
STAGE1 := $(BUILD_DIR)/stage1.bin

OS_IMG := $(BUILD_DIR)/os.img

.PHONY: all run clean

all: $(OS_IMG)

run: 
	qemu-system-x86_64 -drive format=raw,file=$(OS_IMG)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(STAGE0): $(BUILD_DIR) boot/stage0.asm
	nasm -f bin boot/stage0.asm -o $(STAGE0) -DSTAGE1_SECTOR_COUNT=1 # only 1 sector for now

$(STAGE1): $(BUILD_DIR) boot/stage1.asm
	nasm -f bin boot/stage1.asm -o $(STAGE1)

$(OS_IMG): $(STAGE0) $(STAGE1)
	dd if=/dev/zero of=$(OS_IMG) bs=512 count=2880
	dd if=$(STAGE0) of=$(OS_IMG) bs=512 count=1 conv=notrunc
	dd if=$(STAGE1) of=$(OS_IMG) bs=512 count=1 conv=notrunc seek=1

clean:
	rm -rf $(BUILD_DIR)

