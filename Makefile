ASM := nasm
CPP32 := i686-elf-g++
CPP64 := g++
LD32 := i686-elf-ld
LD64 := ld
AR32 := i686-elf-ar
AR64 := ar

ASM_BIN := -f bin
ASM_ELF32 := -f elf32
ASM_ELF64 := -f elf64

CPP_STL_DIR = stl
CPP_COMMON_DIR = common

CPP_FLAGS := -std=c++20 \
						 -fno-unwind-tables \
						 -fno-asynchronous-unwind-tables \
						 -fno-exceptions \
						 -fno-rtti \
						 -ffreestanding \
						 -c \
						 -I$(CPP_STL_DIR) \
						 -I$(CPP_COMMON_DIR)
CPP32_FLAGS := $(CPP_FLAGS) -m32 -DARCH_32
CPP64_FLAGS := $(CPP_FLAGS) -m64 -DARCH_64

BUILD_DIR := build
STAGE2_BUILD_DIR := $(BUILD_DIR)/stage2
KERNEL_BUILD_DIR := $(BUILD_DIR)/kernel
OS_COMMON32_BUILD_DIR := $(BUILD_DIR)/os_common32
OS_COMMON64_BUILD_DIR := $(BUILD_DIR)/os_common64

# Targets
STAGE0 := $(BUILD_DIR)/stage0.bin
STAGE1 := $(BUILD_DIR)/stage1.bin

TRAMPOLINE := $(BUILD_DIR)/trampoline.bin

OS_COMMON32 := $(OS_COMMON32_BUILD_DIR)/os_common32.a
OS_COMMON64 := $(OS_COMMON64_BUILD_DIR)/os_common64.a

STAGE2_STUB := $(STAGE2_BUILD_DIR)/entry.bin
STAGE2 := $(STAGE2_BUILD_DIR)/stage2.bin

KERNEL_STUB := $(KERNEL_BUILD_DIR)/entry.bin
KERNEL := $(KERNEL_BUILD_DIR)/kernel.bin

FILESYSTEM_IMG := $(BUILD_DIR)/filesystem.bin
OS_IMG := $(BUILD_DIR)/os.bin

.PHONY: compile run clean 

compile: $(BUILD_DIR)
	bear --output build/compile_commands.json -- make $(OS_IMG)

run: 
	qemu-system-x86_64 \
		-m 4G \
		-machine pc \
		-drive \
		if=ide,media=disk,format=raw,file=$(OS_IMG) \
		-monitor stdio
		#-device VGA,vgamem_mb=32	

clean:
	rm -rf $(BUILD_DIR)


# Ensure directories exist
$(BUILD_DIR):
	mkdir -p $@

$(STAGE2_BUILD_DIR):
	mkdir -p $@

$(KERNEL_BUILD_DIR):
	mkdir -p $@

$(OS_COMMON32_BUILD_DIR):
	mkdir -p $@

$(OS_COMMON64_BUILD_DIR):
	mkdir -p $@

# 16 bit assembly things
$(STAGE0): boot/stage0.asm | $(BUILD_DIR)
	$(ASM) $(ASM_BIN) $< -o $@ 

$(STAGE1): boot/stage1.asm | $(BUILD_DIR)
	$(ASM) $(ASM_BIN) $< -o $@

$(TRAMPOLINE): boot/stage2/trampoline.asm | $(BUILD_DIR)
	$(ASM) $(ASM_BIN) $< -o $@

# Common library compilation
OS_COMMON_CPP := $(wildcard common/*.cpp)
OS_COMMON32_OBJS := $(patsubst common/%.cpp,$(OS_COMMON32_BUILD_DIR)/%.o,$(OS_COMMON_CPP))
OS_COMMON64_OBJS := $(patsubst common/%.cpp,$(OS_COMMON64_BUILD_DIR)/%.o,$(OS_COMMON_CPP))

# Build common objects for 32-bit
$(OS_COMMON32_BUILD_DIR)/%.o: common/%.cpp | $(OS_COMMON32_BUILD_DIR)
	$(CPP32) $(CPP32_FLAGS) $< -o $@

$(OS_COMMON64_BUILD_DIR)/%.o: common/%.cpp | $(OS_COMMON64_BUILD_DIR)
	$(CPP64) $(CPP64_FLAGS) $< -o $@

# Create static libraries
$(OS_COMMON32): $(OS_COMMON32_OBJS) | $(OS_COMMON32_BUILD_DIR)
	$(AR32) rcs $@ $(OS_COMMON32_OBJS)

$(OS_COMMON64): $(OS_COMMON64_OBJS) | $(OS_COMMON64_BUILD_DIR)
	$(AR64) rcs $@ $(OS_COMMON64_OBJS)

# Stage 2 
STAGE2_CPP := $(wildcard boot/stage2/*.cpp)
STAGE2_OBJS := $(patsubst boot/stage2/%.cpp,$(STAGE2_BUILD_DIR)/%.o,$(STAGE2_CPP))

$(STAGE2_STUB): boot/stage2/entry.asm | $(STAGE2_BUILD_DIR)
	$(ASM) $(ASM_ELF32) $< -o $@

# Build source files
$(STAGE2_BUILD_DIR)/%.o: boot/stage2/%.cpp | $(STAGE2_BUILD_DIR)
	$(CPP32) $(CPP32_FLAGS) $< -o $@ 

# Combine everything
$(STAGE2): $(STAGE2_STUB) $(STAGE2_OBJS) $(OS_COMMON32) | $(STAGE2_BUILD_DIR)
	$(LD32) -T boot/stage2/Linker.ld -m elf_i386 $(STAGE2_STUB) $(STAGE2_OBJS) $(OS_COMMON32) -o $@

# Kernel
KERNEL_CPP := $(wildcard kernel/*.cpp)
KERNEL_OBJS := $(patsubst kernel/%.cpp,$(KERNEL_BUILD_DIR)/%.o,$(KERNEL_CPP))

$(KERNEL_STUB): kernel/_entry.asm | $(KERNEL_BUILD_DIR)
	$(ASM) $(ASM_ELF64) $< -o $@

$(KERNEL_BUILD_DIR)/%.o: kernel/%.cpp | $(KERNEL_BUILD_DIR)
	$(CPP64) $(CPP64_FLAGS) $< -o $@

$(KERNEL): $(KERNEL_STUB) $(KERNEL_OBJS) $(OS_COMMON64) | $(KERNEL_BUILD_DIR)
	$(LD64) -T kernel/Linker.ld $(KERNEL_STUB) $(KERNEL_OBJS) $(OS_COMMON64) -o $@

$(FILESYSTEM_IMG): $(STAGE1) $(STAGE2) $(TRAMPOLINE) $(KERNEL)
	python fs-builder.py \
		$(STAGE1),stage1.bin \
		$(STAGE2),stage2.bin \
		$(TRAMPOLINE),trampoline.bin \
		$(KERNEL),kernel.bin \
		>> $(FILESYSTEM_IMG)

$(OS_IMG): $(STAGE0) $(FILESYSTEM_IMG)
	touch $(OS_IMG)
	cat $(STAGE0) >> $(OS_IMG)
	cat $(FILESYSTEM_IMG) >> $(OS_IMG) 

