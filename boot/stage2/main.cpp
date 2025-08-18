extern "C" void stage2_main() {
	volatile char* vga = (char*)0xB8000;
	vga[0] = 'H';
	vga[1] = 0x07;
	vga[2] = 'i';
	vga[3] = 0x07;
	while (1); // hang
}

void build_page_tables() {

}

