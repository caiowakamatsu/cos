#include "physical_page.hpp"

#include <types.hpp>

namespace cos {

page_bitmap::page_bitmap(std::span<std::byte> bitmap_storage) : bitmap(bitmap_storage) {
	for (auto i = 0; i < bitmap.size(); i++) {
		bitmap[i] = std::byte(0);
	}

	const auto self_address = reinterpret_cast<std::size_t>(bitmap.data());
	const auto page_count = (bitmap.size_bytes() + physical_page::size - 1) / physical_page::size;
	const auto page_start = self_address >> 12;
	for (auto i = 0; i < page_count; i++) {
		set_page({.number = page_start + i}, physical_page_status::used);
	}
}

void page_bitmap::set_page(physical_page page, physical_page_status status) noexcept {
	const auto byte_number = page.number / 8;
	const auto bit = page.number % 8;
	const auto mask = std::byte(1) << bit;

	if (status == physical_page_status::free) {
		bitmap[byte_number] &= ~mask;
	} else {
		bitmap[byte_number] |= mask;
	}
}

physical_page_status page_bitmap::status_of(physical_page page) const noexcept {
	const auto byte_number = page.number / 8;
	const auto bit = page.number % 8;
	const auto mask = std::byte(1) << bit;
	const auto value = (bitmap[byte_number] & mask) >> bit;
	return static_cast<physical_page_status>(value);
}

std::size_t page_bitmap::page_count() const noexcept {
	// Technically this lies, since we might not align to 8 page boundaries
	// But this will also only be an issue if we get to using all of them
	// At which point I presume future me will find a fix for this
	return bitmap.size() * 8;
}

physical_page_allocator::physical_page_allocator(page_bitmap* bitmap) : bitmap(bitmap) {}

std::byte* physical_page_allocator::allocate_pages(std::size_t page_count) {
	for (std::size_t i = 0; i < bitmap->page_count(); i++) {
		auto large_enough = true;

		for (std::size_t j = 0; j < page_count; j++) {
			if (bitmap->status_of({.number = i + j}) != physical_page_status::free) {
				large_enough = false;
				break;
			}
		}

		if (large_enough) {
			for (std::size_t j = 0; j < page_count; j++) {
				bitmap->set_page({.number = i + j}, physical_page_status::used);
			}

			const auto physical_address = i << 12;
			return reinterpret_cast<std::byte*>(physical_address);
		}
	}

	return nullptr;
}

std::byte* physical_page_allocator::allocate_memory(std::size_t memory_size) {
	return allocate_pages((memory_size + physical_page::size - 1) / physical_page::size);
}

decomposed_virtual_address::decomposed_virtual_address(std::uint64_t address)
	: pml4((address >> 39) & 0x1FF),
	  pml3((address >> 30) & 0x1FF),
	  pml2((address >> 21) & 0x1FF),
	  pml1((address >> 12) & 0x1FF) {}

void pmap_single(page_table<ptl4_entry> ptl4_table, cos::physical_page_allocator& allocator,
				 std::uint64_t virtual_address, std::uint64_t physical_page_number) {
	const auto indices = decomposed_virtual_address(virtual_address);

	auto ptl3_table = ptl4_table.get_child_of_or_create(indices.pml4, allocator);
	auto ptl2_table = ptl3_table.get_child_of_or_create(indices.pml3, allocator);
	auto ptl1_table = ptl2_table.get_child_of_or_create(indices.pml2, allocator);
	auto& entry = ptl1_table[indices.pml1];

	entry.present = 1;
	entry.read_write = 1;
	entry.raw_page_number = physical_page_number;
}

void pmap(page_table<ptl4_entry> ptl4_table, cos::physical_page_allocator& allocator,
		  std::uint64_t base_virtual_address, std::uint64_t physical_page_start, std::uint64_t page_count) {
	for (std::uint64_t i = 0; i < page_count; i++) {
		const auto virtual_address = base_virtual_address + i * 4096;
		const auto page_number = physical_page_start + i;
		pmap_single(ptl4_table, allocator, virtual_address, page_number);
	}
}

}  // namespace cos
