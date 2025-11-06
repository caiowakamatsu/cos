#include "page_table.hpp"

namespace {
using namespace kernel::memory;

constexpr auto recursive_slot_index = std::uint64_t(511);

[[nodiscard]] std::uint64_t create_traversable_pointer(std::uint64_t index_0, std::uint64_t index_1,
													   std::uint64_t index_2, std::uint64_t index_3) {
	const std::uint64_t sign_extension = 0xFFFFull << 48;

	return sign_extension | (index_0 << 39) | (index_1 << 30) | (index_2 << 21) | (index_3 << 12);
}

}  // namespace

namespace kernel::memory {

[[nodiscard]] std::span<ptl4_entry> traverse_page_table() {
	return {reinterpret_cast<ptl4_entry*>(::create_traversable_pointer(::recursive_slot_index, ::recursive_slot_index,
																	   ::recursive_slot_index, ::recursive_slot_index)),
			512};
}

[[nodiscard]] std::span<ptl3_entry> traverse_page_table(std::uint16_t ptl4_index) {
	return {reinterpret_cast<ptl3_entry*>(::create_traversable_pointer(::recursive_slot_index, ::recursive_slot_index,
																	   ::recursive_slot_index, ptl4_index)),
			512};
}

[[nodiscard]] std::span<ptl2_entry> traverse_page_table(std::uint16_t ptl4_index, std::uint16_t ptl3_index) {
	return {reinterpret_cast<ptl2_entry*>(
				::create_traversable_pointer(::recursive_slot_index, ::recursive_slot_index, ptl4_index, ptl3_index)),
			512};
}

[[nodiscard]] std::span<ptl1_entry> traverse_page_table(std::uint16_t ptl4_index, std::uint16_t ptl3_index,
														std::uint16_t ptl2_index) {
	return {reinterpret_cast<ptl1_entry*>(
				::create_traversable_pointer(::recursive_slot_index, ptl4_index, ptl3_index, ptl2_index)),
			512};
}

}  // namespace kernel::memory
