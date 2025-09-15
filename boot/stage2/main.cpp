#include <array.hpp>
#include <memory.hpp>
#include <span.hpp>
#include <string.hpp>
#include <types.hpp>

#include "boot_info.hpp"
#include "disk.hpp"
#include "e820.hpp"
#include "filesystem.hpp"
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
	std::size_t byte_size;		   // How many bytes large is the bitmap space?
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
		if (region.type == cos::e820_entry_type::usable && region.length >= bytes_required &&
			region.base_address >= 0x10'0000) {
			address = reinterpret_cast<std::byte *>(static_cast<std::uint32_t>(region.base_address));
			found = true;
			break;
		}
	}

	return {.found = found,
			.storage_address = address,
			.pages_required = required_pages,
			.byte_size = static_cast<std::size_t>(bytes_required)};
}

void initialize_used_pages(cos::terminal &terminal, std::span<const cos::e820_entry> regions, cos::page_bitmap &bitmap,
						   std::size_t max_page) {
	// Update all of the memory regions to be used
	for (auto i = 0; i < regions.size(); i++) {
		const auto &region = regions[i];
		if (region.type != cos::e820_entry_type::usable) {
			const auto base_page_number = region.base_address >> 12;
			const auto page_count = (region.length + cos::physical_page::size - 1) / cos::physical_page::size;
			for (auto page_offset = 0; page_offset < page_count; page_offset++) {
				if (base_page_number + page_offset >= max_page) {
					break;
				}
				bitmap.set_page({.number = base_page_number + page_offset}, cos::physical_page_status::used);
			}
		}
	}

	// Initialize stack
	const auto stack_top_ptr = 0x1FF'FFFFull;
	const auto stack_top_page = stack_top_ptr >> 12;  // Does this work as expected?
	const auto stack_page_count = 16ull;
	const auto stack_bottom = stack_top_page - (stack_page_count - 1);
	for (auto page_offset = 0; page_offset < stack_page_count; page_offset++) {
		bitmap.set_page({.number = stack_bottom + page_offset}, cos::physical_page_status::used);
	}

	// VGA buffer data
	bitmap.set_page({.number = 0xB8000 >> 12}, cos::physical_page_status::used);

	// Memory regions / metadata
	bitmap.set_page({.number = 0x1000 >> 12}, cos::physical_page_status::used);

	// Stage2 code location
	const auto stage2_metadata = cos::filesystem::find("stage2.bin");
	const auto stage2_page_count = (stage2_metadata.sector_count + 3) / 4;
	const auto stage2_start = 0x8000ull >> 12;
	for (auto page_offset = 0; page_offset < stage2_page_count; page_offset++) {
		bitmap.set_page({.number = stage2_start + page_offset}, cos::physical_page_status::used);
	}
}

[[nodiscard]] std::byte *create_page_table(cos::physical_page_allocator &allocator,
										   std::uint32_t kernel_physical_address, std::size_t kernel_page_count,
										   std::uint32_t kernel_stack_physical_address,
										   std::size_t kernel_stack_page_count) {
	auto root_table_allocation = allocator.allocate_pages(1);

	auto root_table = cos::page_table<cos::ptl4_entry>(reinterpret_cast<cos::ptl4_entry *>(root_table_allocation));
	root_table.clear();

	// Identity map the first 32MB
	const auto identity_map_page_count = 32 * 256;	// 256 pages in 1MB
	const auto identity_map_address_start = 0ull;	// Starts at 0x0
	const auto identity_map_page_start = 0ull;		// Starts at the physical page number 0
	pmap(root_table, allocator, identity_map_address_start, identity_map_page_start, identity_map_page_count);

	// Map kernel to higher-half
	const auto kernel_virt_base = 0xFFFFFFFF80000000ull;  // Higher-half kernel base
	pmap(root_table, allocator, kernel_virt_base, kernel_physical_address / 4096, kernel_page_count);

	const auto stack_virt_base = 0xFFFFFFFF7FFF0000ull;	 // Stack base (64KB below kernel)
	pmap(root_table, allocator, stack_virt_base, kernel_stack_physical_address / 4096, kernel_stack_page_count);

	auto recursive_entry = root_table[511];
	recursive_entry.present = 1;
	recursive_entry.read_write = 1;
	recursive_entry.raw_page_number = reinterpret_cast<std::uint32_t>(root_table_allocation) >> 12;

	return root_table_allocation;
}

extern "C" void stage2_main() {
	auto terminal = cos::terminal(reinterpret_cast<char *>(0xB8000));

	const auto memory_regions = std::span<const cos::e820_entry>(reinterpret_cast<cos::e820_entry *>(0x1210),
																 *reinterpret_cast<std::uint16_t *>(0x1200));
	print_memory_regions(terminal, memory_regions);

	const auto [page_bitmap_found, page_bitmap_location, page_count, page_bitmap_size] =
		find_location_for_page_bitmap(memory_regions);
	if (!page_bitmap_found) {
		// Couldn't find a location for it
		terminal << "failed to find location for bitmap with " << cos::decimal(page_count) << " pages\n";
		while (true);
	} else {
		terminal << "bitmap allocated at " << cos::hex(reinterpret_cast<std::uint32_t>(page_bitmap_location))
				 << " total bytes for bitmap " << cos::decimal(page_bitmap_size) << " page count "
				 << cos::decimal(page_count) << "\n";
	}

	auto page_bitmap = cos::page_bitmap({page_bitmap_location, page_bitmap_size});
	initialize_used_pages(terminal, memory_regions, page_bitmap, page_count);
	terminal << "finished writing pages\n";

	const auto trampoline_file_data = cos::filesystem::find("trampoline.bin");
	if (trampoline_file_data.sector_count == 0) {
		terminal << "can't find trampolinein the filesystem\n";
		while (true);
	} else {
		terminal << "found trampoline in the filesystem, sector " << cos::decimal(trampoline_file_data.sector_start)
				 << "\n";
	}

	auto allocator = cos::physical_page_allocator(&page_bitmap);
	// 4 sectors = 1 page
	const auto trampoline_allocation = allocator.allocate_pages((trampoline_file_data.sector_count + 3) / 4);
	if (const auto status = cos::read_from_disk(trampoline_file_data.sector_start + 1,
												trampoline_file_data.sector_count, trampoline_allocation);
		status != 0) {
		terminal << "failed to read trampline into memory " << cos::hex(status) << "\n";
		while (true);
	} else {
		terminal << "successfully loaded trampoline @ "
				 << cos::hex(reinterpret_cast<std::size_t>(trampoline_allocation)) << ", "
				 << cos::decimal(trampoline_file_data.sector_count) << " sectors read\n";
	}

	// Load kernel code
	const auto kernel_file_data = cos::filesystem::find("kernel.bin");
	if (kernel_file_data.sector_count == 0) {
		terminal << "can't find kernel in the filesystem\n";
		while (true);
	} else {
		terminal << "found kernel in the filesystem, sector " << cos::decimal(kernel_file_data.sector_start) << "\n";
	}

	const auto kernel_allocation = allocator.allocate_pages((kernel_file_data.sector_count + 3) / 4);
	if (const auto status =
			cos::read_from_disk(kernel_file_data.sector_start + 1, kernel_file_data.sector_count, kernel_allocation);
		status != 0) {
		terminal << "failed to read kernel into memory " << cos::hex(status) << "\n";
		while (true);
	} else {
		terminal << "successfully loaded kernel @ " << cos::hex(reinterpret_cast<std::size_t>(kernel_allocation))
				 << ", " << cos::decimal(kernel_file_data.sector_count) << " sectors read\n";
	}
	const auto kernel_stack_page_count = 16;
	const auto kernel_stack_allocation = allocator.allocate_pages(kernel_stack_page_count);

	auto page_table_root = create_page_table(
		allocator, reinterpret_cast<std::uint32_t>(kernel_allocation), (kernel_file_data.sector_count + 3) / 4,
		reinterpret_cast<std::uint32_t>(kernel_stack_allocation), kernel_stack_page_count);
	terminal << "created page table @ " << cos::hex(reinterpret_cast<std::uint32_t>(page_table_root)) << "\n";

	// Prepare boot info
	auto boot_info_alloc = allocator.allocate_memory(sizeof(cos::boot_info));
	auto boot_info = reinterpret_cast<cos::boot_info *>(boot_info_alloc);
	boot_info->memory_region_count = memory_regions.size();
	for (std::uint64_t i = 0; i < boot_info->memory_region_count; i++) {
		boot_info->memory_regions[i] = memory_regions[i];
	}

	const auto bitmap_address_uint = reinterpret_cast<std::uint32_t>(page_bitmap_location);
	boot_info->page_bitmap_start = static_cast<std::uint64_t>(bitmap_address_uint);
	boot_info->page_bitmap_count = static_cast<std::uint64_t>(page_count);

	auto trampoline_func = (void (*)()) reinterpret_cast<void *>(trampoline_allocation);
	asm volatile(
		"movl %0, %%eax\n"
		"movl %1, %%ebx\n"
		"call *%2\n"
		:
		: "r"(reinterpret_cast<std::uint32_t>(page_table_root)), "r"(reinterpret_cast<std::uint32_t>(boot_info_alloc)),
		  "r"(trampoline_func)
		: "eax", "ebx");

	while (true);  // hang
}
