#ifndef COS_PAGE_TABLE_ENTRY_HPP
#define COS_PAGE_TABLE_ENTRY_HPP

#include "physical_page.hpp"
#include "terminal.hpp"
#include "types.hpp"
namespace cos {

using qword = unsigned long long;

struct ptl3_entry;
struct ptl4_entry {
	using next_entry = ptl3_entry;
	qword present : 1;
	qword read_write : 1;
	qword user_supervisor : 1;
	qword page_write_through : 1;
	qword page_cache_disable : 1;
	qword accessed : 1;
	qword ignored : 1;
	qword must_be_zero : 2;
	qword available_to_software0 : 3;
	qword raw_page_number : 40;
	qword available_to_software1 : 11;
	qword no_execute : 1;
};

struct ptl2_entry;
struct ptl3_entry {
	using next_entry = ptl2_entry;
	qword present : 1;
	qword read_write : 1;
	qword user_supervisor : 1;
	qword page_write_through : 1;
	qword page_cache_disable : 1;
	qword accessed : 1;
	qword dirty : 1;
	qword page_size : 1;
	qword global : 1;
	qword available_to_software0 : 3;
	qword raw_page_number : 40;
	qword available_to_software1 : 11;
	qword no_execute : 1;
};

struct ptl1_entry;
struct ptl2_entry {
	using next_entry = ptl1_entry;
	qword present : 1;
	qword read_write : 1;
	qword user_supervisor : 1;
	qword page_write_through : 1;
	qword page_cache_disable : 1;
	qword accessed : 1;
	qword dirty : 1;
	qword page_size : 1;
	qword global : 1;
	qword available_to_software0 : 3;
	qword raw_page_number : 40;
	qword available_to_software1 : 11;
	qword no_execute : 1;
};

struct ptl1_entry {
	using next_entry = void;
	qword present : 1;
	qword read_write : 1;
	qword user_supervisor : 1;
	qword page_write_through : 1;
	qword page_cache_disable : 1;
	qword accessed : 1;
	qword dirty : 1;
	qword page_attribute_table : 1;
	qword global : 1;
	qword available_to_software0 : 3;
	qword raw_page_number : 40;
	qword available_to_software1 : 7;
	qword protection_keys : 4;
	qword no_execute : 1;
};

struct physical_page {
	explicit physical_page(cos::uint64_t number) : number(number) {}

	explicit physical_page(cos::byte *physical_address) : number(0) {
		// teChnICallY uSe MemBeR InIt
		const auto addr_int = reinterpret_cast<cos::uint64_t>(physical_address);
		number = addr_int >> 12;
	}

	[[nodiscard]] cos::byte *address() const noexcept { return reinterpret_cast<cos::byte *>(number << 12); }

	cos::uint64_t number;
};

inline void out_debug(cos::uint8_t value) { asm volatile("outb %0, %1" : : "a"(value), "Nd"(cos::uint16_t(0x3F8))); }
template <typename LevelEntryT>
struct page_table {
public:
	LevelEntryT *entries = nullptr;

	page_table(LevelEntryT *entries) : entries(entries) {}

	page_table(cos::byte *entries_address) : entries(reinterpret_cast<LevelEntryT *>(entries_address)) {}

	void clear() {
		for (int i = 0; i < 512; i++) {
			entries[i].present = 0;
		}
	}

	LevelEntryT &operator[](cos::uint64_t index) { return entries[index]; }

	// Physical page number of child table
	// (creates it if it doesn't exist)
	[[nodiscard]] page_table<typename LevelEntryT::next_entry> get_child_of_or_create(cos::uint16_t index,
																					  cos::page_allocator &allocator,
																					  cos::terminal &terminal) {
		if (entries[index].present == 1) {
			// terminal << " found table\n";
			const auto page = physical_page(entries[index].raw_page_number);
			return page_table<typename LevelEntryT::next_entry>(page.address());
		} else {
			const auto physical_address = allocator.allocate_pages(1);
			entries[index].present = 1;
			entries[index].read_write = 1;
			entries[index].raw_page_number = physical_page(physical_address).number;
			terminal << " allocated table @ " << cos::hex(reinterpret_cast<cos::uint64_t>(physical_address)) << "\n";

			auto table = page_table<typename LevelEntryT::next_entry>(physical_address);
			table.clear();

			// terminal << "here\n";

			return table;
		}
	}
};

struct decomposed_virtual_address {
	cos::uint64_t pml4_index;
	cos::uint64_t pml3_index;
	cos::uint64_t pml2_index;
	cos::uint64_t pml1_index;
};
[[nodiscard]] static inline decomposed_virtual_address decompose_virtual_address(cos::byte *address) {
	const auto va = reinterpret_cast<cos::uint64_t>(address);
	return {
		.pml4_index = (va >> 39) & 0x1FF,
		.pml3_index = (va >> 30) & 0x1FF,
		.pml2_index = (va >> 21) & 0x1FF,
		.pml1_index = (va >> 12) & 0x1FF,
	};
}

static inline void pmap_single(page_table<ptl4_entry> ptl4_table, cos::terminal &terminal,
							   cos::page_allocator &allocator, cos::byte *virtual_address,
							   cos::byte *physical_address) {
	const auto [pml4_index, pml3_index, pml2_index, pml1_index] = decompose_virtual_address(virtual_address);
	/*
		terminal << cos::hex(reinterpret_cast<cos::uint64_t>(physical_address)) << ":"
				 << cos::hex(reinterpret_cast<cos::uint64_t>(virtual_address)) << ", " << cos::decimal(pml4_index) << ",
	   "
				 << cos::decimal(pml3_index) << ", " << cos::decimal(pml2_index) << ", " << cos::decimal(pml1_index)
				 << "\n"; */

	auto ptl3_table = ptl4_table.get_child_of_or_create(pml4_index, allocator, terminal);
	auto ptl2_table = ptl3_table.get_child_of_or_create(pml3_index, allocator, terminal);
	auto ptl1_table = ptl2_table.get_child_of_or_create(pml2_index, allocator, terminal);
	auto &entry = ptl1_table[pml1_index];

	entry.present = 1;
	entry.global = 1;
	entry.read_write = 1;
	entry.raw_page_number = physical_page(physical_address).number;
}

// If this isn't page aligned, that's on you tbh
static inline void pmap(page_table<ptl4_entry> root, cos::page_allocator &allocator, cos::byte *virtual_address,
						cos::byte *physical_address, cos::uint32_t length, cos::terminal &terminal) {
	const auto pages_to_map = (cos::uint64_t(length) + 4095) / 4096;
	terminal << "pages to map: " << cos::decimal(pages_to_map) << "\n";
	for (cos::uint64_t i = 0; i < pages_to_map; i++) {
		const auto page_virtual_address = virtual_address + i * 4096;
		const auto page_physical_address = physical_address + i * 4096;
		pmap_single(root, terminal, allocator, page_virtual_address, page_physical_address);
		//  terminal << "Mapping " << cos::hex(reinterpret_cast<cos::uint64_t>(page_physical_address)) << " to "
		//		 << cos::hex(reinterpret_cast<cos::uint64_t>(page_virtual_address)) << "\n";
	}
}

struct page_map_start_data {
	cos::byte *code_data;
	cos::uint64_t code_size;
	cos::byte *stack_data;
	cos::uint64_t stack_size;
	cos::byte *text_data;
	cos::uint64_t text_size;
};
[[nodiscard]] static inline cos::byte *initialize_page_tables(page_map_start_data data, cos::terminal &terminal,
															  cos::page_allocator &allocator) {
	const auto pml4_phys_addr = allocator.allocate_pages(1);
	auto root_table = page_table<ptl4_entry>(reinterpret_cast<ptl4_entry *>(pml4_phys_addr));
	root_table.clear();

	terminal << "pml4 physical address: " << cos::hex(reinterpret_cast<cos::uint64_t>(pml4_phys_addr)) << "\n";

	const auto stack_page_count = (data.stack_size + 4095) / 4096;

	constexpr auto kernel_code_start = 0xFFFF800000000000;
	const auto kernel_stack_start = (510ull - stack_page_count) << 39;
	constexpr auto kernel_text_start = 0xFFFFB00000000000;

	terminal << "Stack size: " << cos::memory_size(data.stack_size) << " page count: " << cos::decimal(stack_page_count)
			 << "\n";
	terminal << "stack start: " << cos::hex(kernel_stack_start) << "\n";
	/*
	pmap(root_table, allocator, reinterpret_cast<cos::byte *>(kernel_code_start), data.code_data, data.code_size,
		 terminal);
	pmap(root_table, allocator, reinterpret_cast<cos::byte *>(kernel_stack_start), data.stack_data, data.stack_size,
		 terminal);
	pmap(root_table, allocator, reinterpret_cast<cos::byte *>(kernel_text_start), data.text_data, data.text_size,
		 terminal);
		 */
	// Map ourselves to ourselves
	terminal << "starting map\n";
	auto fake_terminal = cos::terminal(nullptr);
	pmap(root_table, allocator, reinterpret_cast<cos::byte *>(0x000'000), reinterpret_cast<cos::byte *>(0x0), 0x800'000,
		 terminal);
	terminal << "finished mapping\n";

	return pml4_phys_addr;
}

}  // namespace cos

#endif
