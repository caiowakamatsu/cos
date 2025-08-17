BUILD_DIR := build
STAGE0 := $(BUILD_DIR)/stage0.bin
OS_IMG := $(BUILD_DIR)/os.img

.PHONY: all run clean

all: $(OS_IMG)

run: 
	qemu-system-x86_64 -drive format=raw,file=$(OS_IMG)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(STAGE0): $(BUILD_DIR) boot/stage0.asm
	nasm -f bin boot/stage0.asm -o $(STAGE0)

$(OS_IMG): $(STAGE0)
	dd if=/dev/zero of=$(OS_IMG) bs=512 count=2880
	dd if=$(STAGE0) of=$(OS_IMG) bs=512 count=1 conv=notrunc

clean:
	rm -rf $(BUILD_DIR)

