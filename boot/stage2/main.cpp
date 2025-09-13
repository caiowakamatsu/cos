#include <memory.hpp>
#include <span.hpp>
#include <string.hpp>
#include <types.hpp>

#include "e820.hpp"
#include "physical_page.hpp"
#include "terminal.hpp"

void print_memory_regions(cos::terminal &terminal, std::span<const cos::e820_entry> regions) {
	terminal << "loaded total of " << cos::decimal(regions.size()) << " memory regions from e820\n";
	for (auto i = 0; i < regions.size(); i++) {
		const auto &entry = regions[i];
		terminal << "A: " << cos::hex(entry.base_address) << " L " << cos::memory_size(entry.length);

		terminal << "(";
		switch (entry.type) {
			case cos::e820_entry_type::usable:
				terminal << "usable";
				break;
			case cos::e820_entry_type::reserved:
				terminal << "reserved";
				break;
			case cos::e820_entry_type::acpi_reclaimable:
				terminal << "acpi_reclaimable";
				break;
			case cos::e820_entry_type::acpi_nvs:
				terminal << "acpi_nvs";
				break;
			case cos::e820_entry_type::bad_memory:
				terminal << "bad_memory";
				break;
		}
		terminal << ")\n";
	}
}

struct bitmap_location {
	bool found = false;
	std::byte *storage_address;	   // Where there is enough space for the bitmap
	std::uint64_t pages_required;  // How many pages are required to represent all usable memory?
};
[[nodiscard]] bitmap_location find_location_for_page_bitmap(std::span<const cos::e820_entry> regions) {
	// How much memory can we represent? (Up to max USABLE from regions)
	auto representable_memory_range = std::uint64_t(0);
	for (auto i = 0; i < regions.size(); i++) {
		if (regions[i].type == cos::e820_entry_type::usable) {
			representable_memory_range = regions[i].base_address + regions[i].length;
		}
	}

	// How many pages are required to represent all this memory?
	const auto required_pages = (representable_memory_range + cos::physical_page::size - 1) / cos::physical_page::size;
	const auto bytes_required = (required_pages + 7) / 8;  // Each page is 1 bit

	auto found = false;
	std::byte *address = nullptr;
	for (auto i = 0; i < regions.size(); i++) {
		const auto &region = regions[i];
		// We have to just hope this doesn't interact with any of our existing places in memory
		// This can be made better, but it requires too much work and I want to feed my gambling addiction
		if (region.type == cos::e820_entry_type::usable && region.length >= bytes_required) {
			address = reinterpret_cast<std::byte *>(static_cast<std::uint32_t>(region.base_address));
			found = true;
			break;
		}
	}

	return {.found = found, .storage_address = address, .pages_required = required_pages};
}

extern "C" void stage2_main() {
	auto terminal = cos::terminal(reinterpret_cast<char *>(0xB8000));

	const auto memory_regions = std::span<const cos::e820_entry>(reinterpret_cast<cos::e820_entry *>(0x1210),
																 *reinterpret_cast<std::uint16_t *>(0x1200));
	print_memory_regions(terminal, memory_regions);

	const auto [page_bitmap_found, page_bitmap_location, page_count] = find_location_for_page_bitmap(memory_regions);
	if (!page_bitmap_found) {
		// Couldn't find a location for it
		terminal << "failed to find location for bitmap with " << cos::decimal(page_count) << " pages\n";
		while (true);
	}

	auto page_bitmap = cos::page_bitmap();

	while (true);  // hang
}
