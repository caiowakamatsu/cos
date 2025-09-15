#include <boot_info.hpp>

extern "C" void k_main(cos::boot_info* boot_info) {
	volatile char* vga = reinterpret_cast<char*>(0xB8000);
	vga[0] = 'A';
	vga[2] = sizeof(void*) + '0';
	vga[4] = boot_info->memory_region_count + '0';
	while (true);
}
