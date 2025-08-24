BUILD_DIR := build
STAGE0 := $(BUILD_DIR)/stage0.bin
STAGE1 := $(BUILD_DIR)/stage1.bin
STAGE2_CPP := $(wildcard boot/stage2/*.cpp)
STAGE2_OBJS := $(patsubst boot/stage2/%.cpp,$(BUILD_DIR)/%.o,$(STAGE2_CPP))
STAGE2_ENTRY := $(BUILD_DIR)/stage2_entry.o
STAGE2 := $(BUILD_DIR)/stage2.bin


STL_DIRECTORY := stl

OS_IMG := $(BUILD_DIR)/os.img

.PHONY: all run clean 

all: $(OS_IMG)

compile: $(BUILD_DIR) $(CLEAN)
	bear --output build/compile_commands.json -- make

run: 
	qemu-system-x86_64 -enable-kvm -m 8G -machine pc -cpu host -drive if=ide,media=disk,format=raw,file=$(OS_IMG) -monitor stdio

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(STAGE0): $(BUILD_DIR) boot/stage0.asm
	nasm -f bin boot/stage0.asm -o $(STAGE0) -DSTAGE1_SECTOR_COUNT=1 # only 1 sector for now

$(STAGE1): $(BUILD_DIR) boot/stage1.asm
	nasm -f bin boot/stage1.asm -o $(STAGE1) -DSTAGE2_SECTOR_COUNT=16 -DSTAGE2_SECTOR_START=2

$(BUILD_DIR)/%.o: boot/stage2/%.cpp
	i686-elf-g++ -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-exceptions -fno-rtti -ffreestanding -I$(STL_DIRECTORY) -m32 -c $< -o $@

$(STAGE2_ENTRY): boot/stage2/entry.asm
	nasm -f elf32 boot/stage2/entry.asm -o $(STAGE2_ENTRY)

$(STAGE2): $(STAGE2_ENTRY) $(STAGE2_OBJS) $(STAGE2_LD)
	i686-elf-ld -T boot/stage2/Linker.ld -m elf_i386 $(STAGE2_ENTRY) $(STAGE2_OBJS) -o $@

$(OS_IMG): $(STAGE0) $(STAGE1) $(STAGE2)
	dd if=/dev/zero of=$(OS_IMG) bs=512 count=2880
	dd if=$(STAGE0) of=$(OS_IMG) bs=512 count=1 conv=notrunc
	dd if=$(STAGE1) of=$(OS_IMG) bs=512 count=1 conv=notrunc seek=1
	dd if=$(STAGE2) of=$(OS_IMG) bs=512 count=16 conv=notrunc seek=2

clean:
	rm -rf $(BUILD_DIR)

