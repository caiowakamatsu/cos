#ifndef KERNEL_MEMORY_PAGE_TABLE_HPP
#define KERNEL_MEMORY_PAGE_TABLE_HPP

#include <array.hpp>
#include <span.hpp>
#include <type_traits.hpp>
#include <types.hpp>

namespace kernel::memory {

struct ptl1_entry {
	using next_entry = void;
	std::uint64_t present : 1 = 0;
	std::uint64_t read_write : 1 = 0;
	std::uint64_t user_supervisor : 1 = 0;
	std::uint64_t page_write_through : 1 = 0;
	std::uint64_t page_cache_disable : 1 = 0;
	std::uint64_t accessed : 1 = 0;
	std::uint64_t dirty : 1 = 0;
	std::uint64_t page_attribute_table : 1 = 0;
	std::uint64_t global : 1 = 0;
	std::uint64_t available_to_software0 : 3 = 0;
	std::uint64_t raw_page_number : 40 = 0;
	std::uint64_t available_to_software1 : 7 = 0;
	std::uint64_t protection_keys : 4 = 0;
	std::uint64_t no_execute : 1 = 0;
};

struct ptl2_entry {
	using next_entry = ptl1_entry;
	std::uint64_t present : 1 = 0;
	std::uint64_t read_write : 1 = 0;
	std::uint64_t user_supervisor : 1 = 0;
	std::uint64_t page_write_through : 1 = 0;
	std::uint64_t page_cache_disable : 1 = 0;
	std::uint64_t accessed : 1 = 0;
	std::uint64_t dirty : 1 = 0;
	std::uint64_t page_size : 1 = 0;
	std::uint64_t global : 1 = 0;
	std::uint64_t available_to_software0 : 3 = 0;
	std::uint64_t raw_page_number : 40 = 0;
	std::uint64_t available_to_software1 : 11 = 0;
	std::uint64_t no_execute : 1 = 0;
};

struct ptl3_entry {
	using next_entry = ptl2_entry;
	std::uint64_t present : 1 = 0;
	std::uint64_t read_write : 1 = 0;
	std::uint64_t user_supervisor : 1 = 0;
	std::uint64_t page_write_through : 1 = 0;
	std::uint64_t page_cache_disable : 1 = 0;
	std::uint64_t accessed : 1 = 0;
	std::uint64_t dirty : 1 = 0;
	std::uint64_t page_size : 1 = 0;
	std::uint64_t global : 1 = 0;
	std::uint64_t available_to_software0 : 3 = 0;
	std::uint64_t raw_page_number : 40 = 0;
	std::uint64_t available_to_software1 : 11 = 0;
	std::uint64_t no_execute : 1 = 0;
};

struct ptl4_entry {
	using next_entry = ptl3_entry;
	std::uint64_t present : 1 = 0;
	std::uint64_t read_write : 1 = 0;
	std::uint64_t user_supervisor : 1 = 0;
	std::uint64_t page_write_through : 1 = 0;
	std::uint64_t page_cache_disable : 1 = 0;
	std::uint64_t accessed : 1 = 0;
	std::uint64_t ignored : 1 = 0;
	std::uint64_t must_be_zero : 2 = 0;
	std::uint64_t available_to_software0 : 3 = 0;
	std::uint64_t raw_page_number : 40 = 0;
	std::uint64_t available_to_software1 : 11 = 0;
	std::uint64_t no_execute : 1 = 0;
};

// Uses the recursive mapping to get the root table
[[nodiscard]] std::span<ptl4_entry> traverse_page_table();

[[nodiscard]] std::span<ptl3_entry> traverse_page_table(std::uint16_t ptl4_index);

[[nodiscard]] std::span<ptl2_entry> traverse_page_table(std::uint16_t ptl4_index, std::uint16_t ptl3_index);

[[nodiscard]] std::span<ptl1_entry> traverse_page_table(std::uint16_t ptl4_index, std::uint16_t ptl3_index,
														std::uint16_t ptl2_index);
}  // namespace kernel::memory

#endif
