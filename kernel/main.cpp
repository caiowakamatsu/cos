#include <bits.hpp>
#include <boot_info.hpp>
#include <memory.hpp>
#include <span.hpp>

#include "memory/page_table.hpp"
#include "terminal.hpp"

extern "C" void k_main(cos::boot_info* boot_info) {
	auto terminal = cos::terminal(reinterpret_cast<char*>(0xB8000));

	terminal << "Setting up 64 bit mode\n";

	terminal << "Memory region count: " << cos::decimal(boot_info->memory_region_count) << "\n";
	terminal << "Page bitmap starts at: " << cos::hex(boot_info->page_bitmap_start)
			 << " count: " << cos::decimal(boot_info->page_bitmap_count) << "\n";

	const std::uint64_t slot = 510;
	const std::uint64_t sign_extension = 0xFFFFull << 48;

	const std::uint64_t recursive_addr = sign_extension | (slot << 39) | (slot << 30) | (slot << 21) | (slot << 12);
	terminal << cos::hex(recursive_addr) << "\n";
	terminal << cos::hex(sign_extension) << "\n";

	using namespace kernel::memory;

	auto highest_table = std::span<ptl4_entry>(reinterpret_cast<ptl4_entry*>(recursive_addr), 512);

	for (int i = 0; i < 512; i++) {
		const auto raw_value = std::bit_cast<std::uint64_t>(highest_table[i]);
		const auto is_present = bool(highest_table[i].present);
		if (is_present) {
			terminal << cos::decimal(i) << "is present: " << cos::hex(raw_value) << "\n";
		}
	}
}
