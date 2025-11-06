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

	const auto highest_table = kernel::memory::traverse_page_table();

	for (int i = 0; i < 512; i++) {
		const auto raw_value = std::bit_cast<std::uint64_t>(highest_table[i]);
		const auto is_present = bool(highest_table[i].present);
		if (is_present) {
			terminal << cos::decimal(i) << "is present: " << cos::hex(raw_value) << "\n";

			const auto table3 = kernel::memory::traverse_page_table(i);
			for (int j = 0; j < 512; j++) {
				const auto raw_value_3 = std::bit_cast<std::uint64_t>(table3[j]);
				if (table3[j].present) {
					terminal << cos::decimal(i) << "is present (root:" << cos::decimal(i) << ")"
							 << cos::hex(raw_value_3) << "\n";
				}
			}
		}
	}
}
