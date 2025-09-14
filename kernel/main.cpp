
extern "C" void k_main() {
	volatile char* vga = reinterpret_cast<char*>(0xB8000);
	vga[0] = 'A';
	vga[2] = sizeof(void*) + '0';
	while (true);
}
