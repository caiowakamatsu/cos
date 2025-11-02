#include <boot_info.hpp>
#include <physical_page.hpp>

#include "asm/cr3.hpp"
#include "terminal.hpp"

extern "C" void k_main(cos::boot_info* boot_info) {
	auto terminal = cos::terminal(reinterpret_cast<char*>(0xB8000));

	terminal << "Setting up 64 bit mode\n";

	terminal << "Memory region count: " << cos::decimal(boot_info->memory_region_count) << "\n";
	terminal << "Page bitmap starts at: " << cos::hex(boot_info->page_bitmap_start)
			 << " count: " << cos::decimal(boot_info->page_bitmap_count) << "\n";

	const auto cr3_val = kernel::intrinsic::cr3();
	terminal << "CR3=" << cos::hex(cr3_val) << "\n";

	const std::uint64_t slot = 510;
	const std::uint64_t sign_extension = 0xFFFFull << 48;

	const std::uint64_t recursive_addr = sign_extension | (slot << 39) | (slot << 30) | (slot << 21) | (slot << 12);
	terminal << cos::hex(recursive_addr) << "\n";
	terminal << cos::hex(sign_extension) << "\n";

	const volatile auto ptr = reinterpret_cast<std::uint64_t*>(recursive_addr);

	auto val = ptr[510];
	terminal << cos::hex(val) << "\n";
}
