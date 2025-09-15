#ifndef COS_PHYSICAL_PAGE_HPP
#define COS_PHYSICAL_PAGE_HPP

#include <span.hpp>
#include <types.hpp>

namespace cos {

struct physical_page {
	std::uint64_t number;

	constexpr static auto size = std::size_t(4096);
};

template <typename T>
T align_up(T value, std::uint64_t alignment) {
	// This implementation works when alignment is a power of 2
	return (value + alignment - 1) & ~(alignment - 1);
}

enum class physical_page_status : std::uint8_t {
	free = 0x0,
	used = 0x1,
};

class page_bitmap {
public:
	explicit page_bitmap(std::span<std::byte> bitmap_storage);

	void set_page(physical_page page, physical_page_status status) noexcept;

	[[nodiscard]] physical_page_status status_of(physical_page page) const noexcept;

	[[nodiscard]] std::size_t page_count() const noexcept;

private:
	std::span<std::byte> bitmap;
};

class physical_page_allocator {
public:
	explicit physical_page_allocator(page_bitmap* bitmap);

	[[nodiscard]] std::byte* allocate_pages(std::size_t page_count);

	[[nodiscard]] std::byte* allocate_memory(std::size_t memory_size);

private:
	page_bitmap* bitmap;
};

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

struct decomposed_virtual_address {
	explicit decomposed_virtual_address(std::uint64_t address);

	std::uint16_t pml4;
	std::uint16_t pml3;
	std::uint16_t pml2;
	std::uint16_t pml1;
};

template <typename PageEntryT>
struct page_table {
public:
	page_table(PageEntryT* entries) : entries(entries) {}

	void clear() {
		for (int i = 0; i < 512; i++) {
			entries[i] = PageEntryT();
		}
	}

	[[nodiscard]] PageEntryT& operator[](std::uint16_t index) { return entries[index]; }

	[[nodiscard]] page_table<typename PageEntryT::next_entry> get_child_of_or_create(
		std::uint16_t index, cos::physical_page_allocator& allocator) {
		if (entries[index].present == 1) {
			// Entry already exists, return the next table
			const auto page_number = static_cast<std::uint32_t>(entries[index].raw_page_number);
			const auto table_address = page_number << 12;
			const auto table_ptr = reinterpret_cast<typename PageEntryT::next_entry*>(table_address);
			return page_table<typename PageEntryT::next_entry>(table_ptr);
		} else {
			const auto table_allocation_ptr = allocator.allocate_pages(1);
			entries[index].present = 1;
			entries[index].read_write = 1;
			entries[index].raw_page_number = reinterpret_cast<std::size_t>(table_allocation_ptr) >> 12;

			const auto table_ptr = reinterpret_cast<typename PageEntryT::next_entry*>(table_allocation_ptr);
			auto table = page_table<typename PageEntryT::next_entry>(table_ptr);
			table.clear();	// Might have bad memory, make sure it's cleared out

			return table;
		}
	}

private:
	PageEntryT* entries;
};

void pmap_single(page_table<ptl4_entry> ptl4_table, cos::physical_page_allocator& allocator,
				 std::uint64_t virtual_address, std::uint64_t physical_page_number);

void pmap(page_table<ptl4_entry> ptl4_table, cos::physical_page_allocator& allocator,
		  std::uint64_t base_virtual_address, std::uint64_t physical_page_start, std::uint64_t page_count);

}  // namespace cos

#endif
